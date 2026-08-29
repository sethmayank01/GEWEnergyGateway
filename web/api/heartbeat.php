<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_REQUEST_BYTES = 4096;

function respond(int $code, array $body): void
{
    http_response_code($code);
    echo json_encode($body, JSON_UNESCAPED_SLASHES);
    exit;
}

function fail(int $code, string $message): void
{
    respond($code, ["status" => "ERROR", "message" => $message]);
}

function pendingCommand(mysqli $conn, string $gatewayId): array
{
    $stmt = $conn->prepare(
        "UPDATE gateway_commands
         SET status = 'EXPIRED', completed_at = COALESCE(completed_at, NOW())
         WHERE gateway_id = ? AND status = 'PENDING'
           AND expires_at IS NOT NULL AND expires_at <= NOW()"
    );
    if ($stmt)
    {
        $stmt->bind_param("s", $gatewayId);
        $stmt->execute();
        $stmt->close();
    }

    $stmt = $conn->prepare(
        "SELECT id, command, parameters
         FROM gateway_commands
         WHERE gateway_id = ? AND status = 'PENDING'
           AND (expires_at IS NULL OR expires_at > NOW())
         ORDER BY id ASC LIMIT 1"
    );
    if (!$stmt)
        return [];
    $stmt->bind_param("s", $gatewayId);
    if (!$stmt->execute())
    {
        error_log("Heartbeat command select failed: " . $stmt->error);
        $stmt->close();
        return [];
    }

    $result = $stmt->get_result();
    if ($result->num_rows !== 1)
    {
        $stmt->close();
        return [];
    }
    $row = $result->fetch_assoc();
    $stmt->close();

    $parameters = new stdClass();
    if (!empty($row["parameters"]))
    {
        $decoded = json_decode($row["parameters"], true);
        if (is_array($decoded))
            $parameters = $decoded;
    }

    $commandId = (int)$row["id"];
    $stmt = $conn->prepare(
        "UPDATE gateway_commands
         SET status = 'DELIVERED', delivered_at = COALESCE(delivered_at, NOW())
         WHERE id = ? AND gateway_id = ? AND status = 'PENDING'"
    );
    if (!$stmt)
        return [];
    $stmt->bind_param("is", $commandId, $gatewayId);
    if (!$stmt->execute() || $stmt->affected_rows !== 1)
    {
        $stmt->close();
        return [];
    }
    $stmt->close();

    return [[
        "id" => $commandId,
        "command" => strtoupper(trim((string)$row["command"])),
        "parameters" => $parameters
    ]];
}

set_exception_handler(function (Throwable $e): void
{
    error_log("HEARTBEAT API UNHANDLED EXCEPTION: " . $e->getMessage());
    fail(500, "Internal server error");
});

if ($_SERVER["REQUEST_METHOD"] !== "POST")
{
    header("Allow: POST");
    fail(405, "Only POST allowed");
}

$length = (int)($_SERVER["CONTENT_LENGTH"] ?? 0);
if ($length <= 0 || $length > MAX_REQUEST_BYTES)
    fail(413, "Invalid request size");
$raw = file_get_contents("php://input", false, null, 0, MAX_REQUEST_BYTES + 1);
if ($raw === false || strlen($raw) > MAX_REQUEST_BYTES)
    fail(413, "Invalid request size");
$data = json_decode($raw, true);
if (!is_array($data))
    fail(400, "Invalid JSON");

foreach (["gatewayId", "apiKey", "firmware", "meterConnected",
          "pendingUploads", "wifiRssi", "freeHeap", "uptimeSeconds"] as $field)
{
    if (!array_key_exists($field, $data))
        fail(400, "Missing field: " . $field);
}

$gatewayId = is_string($data["gatewayId"]) ? trim($data["gatewayId"]) : "";
$apiKey = is_string($data["apiKey"]) ? $data["apiKey"] : "";
$firmware = is_string($data["firmware"]) ? trim($data["firmware"]) : "";
$meterConnected = filter_var($data["meterConnected"], FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE);
$pendingUploads = is_numeric($data["pendingUploads"]) ? (int)$data["pendingUploads"] : -1;
$wifiRssi = is_numeric($data["wifiRssi"]) ? (int)$data["wifiRssi"] : 1;
$freeHeap = is_numeric($data["freeHeap"]) ? (int)$data["freeHeap"] : -1;
$uptimeSeconds = is_numeric($data["uptimeSeconds"]) ? (int)$data["uptimeSeconds"] : -1;

if ($gatewayId === "" || $apiKey === "" || $firmware === "" ||
    $meterConnected === null || $pendingUploads < 0 ||
    $wifiRssi < -127 || $wifiRssi > 0 || $freeHeap < 0 || $uptimeSeconds < 0 ||
    strlen($gatewayId) > 32 || strlen($apiKey) > 255 || strlen($firmware) > 100)
{
    fail(400, "Invalid heartbeat data");
}

$stmt = $conn->prepare(
    "SELECT gateway_id FROM gateways
     WHERE gateway_id = ? AND api_key = ? LIMIT 1"
);
if (!$stmt)
    fail(500, "Database error");
$stmt->bind_param("ss", $gatewayId, $apiKey);
if (!$stmt->execute())
    fail(500, "Database error");
if ($stmt->get_result()->num_rows !== 1)
    fail(401, "Invalid gateway credentials");
$stmt->close();

$meterStatus = $meterConnected ? "CONNECTED" : "DISCONNECTED";
$stmt = $conn->prepare(
    "UPDATE gateways SET
        last_seen = NOW(), last_heartbeat = NOW(), status = 'ONLINE',
        firmware = ?, meter_status = ?, pending_uploads = ?,
        wifi_rssi = ?, free_heap = ?, uptime_seconds = ?
     WHERE gateway_id = ?"
);
if (!$stmt)
{
    error_log("Heartbeat update prepare failed: " . $conn->error);
    fail(500, "Heartbeat database columns are not installed");
}
$stmt->bind_param(
    "ssiiiis",
    $firmware,
    $meterStatus,
    $pendingUploads,
    $wifiRssi,
    $freeHeap,
    $uptimeSeconds,
    $gatewayId
);
if (!$stmt->execute())
{
    error_log("Heartbeat update failed: " . $stmt->error);
    fail(500, "Heartbeat update failed");
}
$stmt->close();

respond(200, [
    "status" => "OK",
    "message" => "Heartbeat recorded",
    "serverTime" => time(),
    "commands" => pendingCommand($conn, $gatewayId)
]);


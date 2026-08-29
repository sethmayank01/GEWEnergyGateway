<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_REQUEST_BYTES = 4096;

function respond(int $httpCode, array $body): void
{
    http_response_code($httpCode);
    echo json_encode($body, JSON_UNESCAPED_SLASHES);
    exit;
}

function fail(int $httpCode, string $message): void
{
    respond($httpCode, ["status" => "ERROR", "message" => $message]);
}

set_exception_handler(function (Throwable $e): void
{
    error_log("COMMAND ACK UNHANDLED EXCEPTION: " . $e->getMessage());
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

foreach (["gatewayId", "apiKey", "commandId", "status"] as $field)
{
    if (!array_key_exists($field, $data))
        fail(400, "Missing field: " . $field);
}

$gatewayId = is_string($data["gatewayId"]) ? trim($data["gatewayId"]) : "";
$apiKey = is_string($data["apiKey"]) ? $data["apiKey"] : "";
$commandId = is_numeric($data["commandId"]) ? (int)$data["commandId"] : 0;
$status = is_string($data["status"]) ? strtoupper(trim($data["status"])) : "";
$message = isset($data["message"]) && is_string($data["message"])
    ? trim($data["message"])
    : "";

if ($gatewayId === "" || $apiKey === "" || $commandId <= 0 || strlen($message) > 500)
    fail(400, "Invalid command acknowledgement");

if (!in_array($status, ["RUNNING", "SUCCESS", "FAILED"], true))
    fail(400, "Invalid command status");

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

if ($status === "RUNNING")
{
    $stmt = $conn->prepare(
        "UPDATE gateway_commands
         SET status = 'RUNNING', started_at = COALESCE(started_at, NOW()),
             result_message = ?
         WHERE id = ? AND gateway_id = ?
           AND status IN ('DELIVERED', 'RUNNING')"
    );
}
else
{
    // Accept a terminal result directly from DELIVERED as well: execution may
    // succeed even if the earlier RUNNING acknowledgement was lost.
    $stmt = $conn->prepare(
        "UPDATE gateway_commands
         SET status = ?, result_message = ?, completed_at = COALESCE(completed_at, NOW())
         WHERE id = ? AND gateway_id = ?
           AND status IN ('DELIVERED', 'RUNNING', 'SUCCESS', 'FAILED')"
    );
}

if (!$stmt)
    fail(500, "Database error");

if ($status === "RUNNING")
    $stmt->bind_param("sis", $message, $commandId, $gatewayId);
else
    $stmt->bind_param("ssis", $status, $message, $commandId, $gatewayId);

if (!$stmt->execute())
{
    error_log("Command acknowledgement update failed: " . $stmt->error);
    fail(500, "Database update failed");
}

if ($stmt->affected_rows < 1)
    fail(409, "Command is not available for this status transition");

respond(200, [
    "status" => "OK",
    "message" => "Command status recorded",
    "commandId" => $commandId,
    "commandStatus" => $status
]);


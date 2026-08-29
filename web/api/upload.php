<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_REQUEST_BYTES = 32768;

function jsonResponse(int $httpCode, array $body): void
{
    http_response_code($httpCode);
    echo json_encode($body, JSON_UNESCAPED_SLASHES);
    exit;
}

function jsonError(int $httpCode, string $message, array $extra = []): void
{
    jsonResponse($httpCode, array_merge([
        "status" => "ERROR",
        "message" => $message
    ], $extra));
}

function fetchGatewayCommands(mysqli $conn, string $gatewayId): array
{
    // Expired commands remain in the database for audit/history.
    $stmt = $conn->prepare(
        "UPDATE gateway_commands
         SET status = 'EXPIRED', completed_at = COALESCE(completed_at, NOW())
         WHERE gateway_id = ?
           AND status = 'PENDING'
           AND expires_at IS NOT NULL
           AND expires_at <= NOW()"
    );

    if ($stmt)
    {
        $stmt->bind_param("s", $gatewayId);
        if (!$stmt->execute())
        {
            error_log("Command expiry update failed: " . $stmt->error);
        }
        $stmt->close();
    }

    // Return only one command. Until device acknowledgements are implemented,
    // select PENDING only so the current firmware cannot execute SEND_LOGS on
    // every upload. DELIVERED redelivery will be enabled with ID deduplication.
    $stmt = $conn->prepare(
        "SELECT id, command, parameters, status
         FROM gateway_commands
         WHERE gateway_id = ?
           AND status = 'PENDING'
           AND (expires_at IS NULL OR expires_at > NOW())
         ORDER BY id ASC
         LIMIT 1"
    );

    if (!$stmt)
    {
        error_log("Command select prepare failed: " . $conn->error);
        return [];
    }

    $stmt->bind_param("s", $gatewayId);
    if (!$stmt->execute())
    {
        error_log("Command select failed: " . $stmt->error);
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
        {
            $parameters = $decoded;
        }
        else
        {
            error_log("Invalid command parameters JSON for command ID " . $row["id"]);
        }
    }

    if ($row["status"] === "PENDING")
    {
        $commandId = (int)$row["id"];
        $update = $conn->prepare(
            "UPDATE gateway_commands
             SET status = 'DELIVERED', delivered_at = COALESCE(delivered_at, NOW())
             WHERE id = ? AND gateway_id = ? AND status = 'PENDING'"
        );

        if ($update)
        {
            $update->bind_param("is", $commandId, $gatewayId);
            if (!$update->execute())
            {
                error_log("Command delivery update failed: " . $update->error);
            }
            $update->close();
        }
    }

    return [[
        "id" => (int)$row["id"],
        "command" => strtoupper(trim((string)$row["command"])),
        "parameters" => $parameters
    ]];
}

set_exception_handler(function (Throwable $e): void
{
    error_log("UPLOAD API UNHANDLED EXCEPTION: " . $e->getMessage());
    jsonError(500, "Internal server error");
});

if ($_SERVER["REQUEST_METHOD"] !== "POST")
{
    header("Allow: POST");
    jsonError(405, "Only POST allowed");
}

$contentLength = (int)($_SERVER["CONTENT_LENGTH"] ?? 0);
if ($contentLength <= 0 || $contentLength > MAX_REQUEST_BYTES)
{
    jsonError(413, "Invalid request size");
}

$input = file_get_contents("php://input", false, null, 0, MAX_REQUEST_BYTES + 1);
if ($input === false || strlen($input) > MAX_REQUEST_BYTES)
{
    jsonError(413, "Invalid request size");
}

$data = json_decode($input, true);
if (!is_array($data))
{
    jsonError(400, "Invalid JSON");
}

$required = ["gatewayId", "apiKey", "sequence", "timestamp", "measurements"];
foreach ($required as $field)
{
    if (!array_key_exists($field, $data))
    {
        jsonError(400, "Missing field: " . $field);
    }
}

if (!is_string($data["gatewayId"]) || trim($data["gatewayId"]) === "" ||
    !is_string($data["apiKey"]) || $data["apiKey"] === "" ||
    !is_numeric($data["sequence"]) || !is_numeric($data["timestamp"]) ||
    !is_array($data["measurements"]))
{
    jsonError(400, "Invalid field types");
}

$gatewayId = trim($data["gatewayId"]);
$apiKey = $data["apiKey"];
$sequence = (int)$data["sequence"];
$timestamp = (int)$data["timestamp"];
$firmware = isset($data["firmware"]) && is_string($data["firmware"])
    ? trim($data["firmware"])
    : null;

if ($sequence < 0 || $timestamp <= 0 || strlen($gatewayId) > 32 ||
    strlen($apiKey) > 255 || ($firmware !== null && strlen($firmware) > 100))
{
    jsonError(400, "Invalid gateway data");
}

$stmt = $conn->prepare(
    "SELECT gateway_id
     FROM gateways
     WHERE gateway_id = ? AND api_key = ?
     LIMIT 1"
);

if (!$stmt)
{
    error_log("Gateway authentication prepare failed: " . $conn->error);
    jsonError(500, "Database error");
}

$stmt->bind_param("ss", $gatewayId, $apiKey);
if (!$stmt->execute())
{
    error_log("Gateway authentication failed: " . $stmt->error);
    jsonError(500, "Database error");
}

$result = $stmt->get_result();
if ($result->num_rows !== 1)
{
    jsonError(401, "Invalid gateway credentials");
}
$stmt->close();

$readingTime = date("Y-m-d H:i:s", $timestamp);
$payload = json_encode($data["measurements"], JSON_UNESCAPED_SLASHES);
if ($payload === false)
{
    jsonError(400, "Invalid measurements");
}

$duplicate = false;
$stmt = $conn->prepare(
    "INSERT INTO meter_readings
        (gateway_id, sequence, reading_time, firmware, data)
     VALUES (?, ?, ?, ?, ?)"
);

if (!$stmt)
{
    error_log("Reading insert prepare failed: " . $conn->error);
    jsonError(500, "Database insert failed");
}

$stmt->bind_param("sisss", $gatewayId, $sequence, $readingTime, $firmware, $payload);
$success = $stmt->execute();
if (!$success)
{
    if ($stmt->errno === 1062)
    {
        $duplicate = true;
    }
    else
    {
        error_log("Database insert error: " . $stmt->errno . " - " . $stmt->error);
        jsonError(500, "Database insert failed");
    }
}
$stmt->close();

$stmt = $conn->prepare(
    "UPDATE gateways
     SET last_seen = NOW(),
         last_sequence = GREATEST(COALESCE(last_sequence, 0), ?),
         firmware = ?, status = 'ONLINE'
     WHERE gateway_id = ?"
);

if ($stmt)
{
    $stmt->bind_param("iss", $sequence, $firmware, $gatewayId);
    if (!$stmt->execute())
    {
        error_log("Gateway status update failed: " . $stmt->error);
    }
    $stmt->close();
}
else
{
    error_log("Gateway status update prepare failed: " . $conn->error);
}

// Commands are returned even for duplicate readings. A lost upload response
// must not prevent a gateway from receiving its pending command.
$commands = fetchGatewayCommands($conn, $gatewayId);

jsonResponse(200, [
    "status" => "OK",
    "message" => $duplicate ? "Duplicate packet ignored" : "Reading stored",
    "gatewayId" => $gatewayId,
    "sequence" => $sequence,
    "timestamp" => time(),
    "commands" => $commands
]);

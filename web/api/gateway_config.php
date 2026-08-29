<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_REQUEST_BYTES = 4096;
const CLOUD_UPLOAD_URL = "https://www.geworks.co.in/energy/api/upload.php";

function respond(int $code, array $body): void
{
    http_response_code($code);
    echo json_encode($body, JSON_UNESCAPED_SLASHES);
    exit;
}

function fail(int $code, string $message): void
{
    respond($code, ["status" => "ERROR", "success" => false, "message" => $message]);
}

set_exception_handler(function (Throwable $e): void
{
    error_log("GATEWAY CONFIG UNHANDLED EXCEPTION: " . $e->getMessage());
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

$gatewayId = isset($data["gatewayId"]) && is_string($data["gatewayId"])
    ? trim($data["gatewayId"]) : "";
$apiKey = isset($data["apiKey"]) && is_string($data["apiKey"])
    ? $data["apiKey"] : "";
$commandId = isset($data["commandId"]) && is_numeric($data["commandId"])
    ? (int)$data["commandId"] : 0;
if ($gatewayId === "" || $apiKey === "" || $commandId <= 0)
    fail(400, "Invalid configuration request");

$stmt = $conn->prepare(
    "SELECT
        g.gateway_id, g.api_key, g.hardware, g.config_version,
        g.last_sequence, g.meter_manufacturer, g.meter_model,
        g.meter_baud, g.meter_parity, g.meter_stop_bits,
        g.meter_slave_id, g.upload_interval
     FROM gateways g
     INNER JOIN gateway_commands c ON c.gateway_id = g.gateway_id
     WHERE g.gateway_id = ? AND g.api_key = ?
       AND c.id = ? AND c.command = 'UPDATE_CONFIG'
       AND c.status IN ('DELIVERED', 'RUNNING')
       AND (c.expires_at IS NULL OR c.expires_at > NOW())
     LIMIT 1"
);
if (!$stmt)
{
    error_log("Gateway config prepare failed: " . $conn->error);
    fail(500, "Database error");
}
$stmt->bind_param("ssi", $gatewayId, $apiKey, $commandId);
if (!$stmt->execute())
    fail(500, "Database error");
$result = $stmt->get_result();
if ($result->num_rows !== 1)
    fail(404, "Configuration command not available");
$gateway = $result->fetch_assoc();
$stmt->close();

$configuration = [
    "gateway" => [
        "gatewayId" => $gateway["gateway_id"],
        "apiKey" => $gateway["api_key"],
        "firmware" => "managed-by-running-image",
        "hardware" => $gateway["hardware"],
        "lastSequence" => (int)($gateway["last_sequence"] ?? 0)
    ],
    "meter" => [
        "manufacturer" => $gateway["meter_manufacturer"],
        "model" => $gateway["meter_model"],
        "port" => "UART2",
        "baud" => (int)$gateway["meter_baud"],
        "parity" => $gateway["meter_parity"],
        "stopBits" => (int)$gateway["meter_stop_bits"],
        "slaveId" => (int)$gateway["meter_slave_id"]
    ],
    "cloud" => [
        "url" => CLOUD_UPLOAD_URL,
        "uploadInterval" => (int)$gateway["upload_interval"]
    ]
];

respond(200, [
    "status" => "OK",
    "success" => true,
    "configurationVersion" => (int)$gateway["config_version"],
    "configuration" => $configuration
]);


<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_REQUEST_BYTES = 4096;
const CLOUD_UPLOAD_URL = "https://www.geworks.co.in/energy/api/upload.php";

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
        "success" => false,
        "message" => $message
    ], $extra));
}

function rejectProvisioning(): void
{
    // Do not reveal which identity value was incorrect.
    jsonError(401, "Device authentication failed");
}

set_exception_handler(function (Throwable $e): void
{
    error_log("PROVISION API UNHANDLED EXCEPTION: " . $e->getMessage());
    jsonError(500, "Internal server error");
});

// ---------------------------------------
// Allow only POST
// ---------------------------------------
if ($_SERVER["REQUEST_METHOD"] !== "POST")
{
    header("Allow: POST");
    jsonError(405, "Only POST allowed");
}

// ---------------------------------------
// Read size-limited JSON body
// ---------------------------------------
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

// ---------------------------------------
// Validate required device fields
// ---------------------------------------
$required = [
    "gatewayId",
    "macAddress",
    "bootstrapSecret",
    "firmware",
    "hardware"
];

foreach ($required as $field)
{
    if (!isset($data[$field]) ||
        !is_string($data[$field]) ||
        trim($data[$field]) === "")
    {
        jsonError(400, "Missing or invalid field: " . $field);
    }
}

$gatewayId = trim($data["gatewayId"]);
$macAddress = strtoupper(trim($data["macAddress"]));
$bootstrapSecret = $data["bootstrapSecret"];
$firmware = trim($data["firmware"]);
$hardware = trim($data["hardware"]);
$wifiNetworks = $data["wifiNetworks"] ?? [];

if (!preg_match('/^GEW[0-9]{6,}$/', $gatewayId))
{
    jsonError(400, "Invalid gatewayId format");
}

if (!filter_var($macAddress, FILTER_VALIDATE_MAC))
{
    jsonError(400, "Invalid macAddress format");
}

if (strlen($bootstrapSecret) < 16 || strlen($bootstrapSecret) > 256)
{
    jsonError(400, "Invalid bootstrapSecret format");
}

if (strlen($firmware) > 100 || strlen($hardware) > 100)
{
    jsonError(400, "Invalid device metadata");
}

if (!is_array($wifiNetworks) || count($wifiNetworks) > 8)
{
    jsonError(400, "Invalid wifiNetworks");
}

$validatedWifiNetworks = [];
foreach ($wifiNetworks as $ssid)
{
    if (!is_string($ssid) || trim($ssid) === "" || strlen($ssid) > 32)
    {
        jsonError(400, "Invalid Wi-Fi network name");
    }

    $ssid = trim($ssid);
    if (!in_array($ssid, $validatedWifiNetworks, true))
    {
        $validatedWifiNetworks[] = $ssid;
    }
}
$wifiNetworksJson = json_encode(
    $validatedWifiNetworks,
    JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES
);

// ---------------------------------------
// Find and authenticate gateway
// ---------------------------------------
$stmt = $conn->prepare(
"
SELECT
    gateway_id,
    api_key,
    bootstrap_secret_hash,
    config_version,
    last_sequence,
    meter_manufacturer,
    meter_model,
    meter_baud,
    meter_parity,
    meter_stop_bits,
    meter_slave_id,
    upload_interval
FROM gateways
WHERE gateway_id = ?
AND mac_address = ?
LIMIT 1
"
);

if (!$stmt)
{
    error_log("Provision query prepare failed: " . $conn->error);
    jsonError(500, "Provisioning database error");
}

$stmt->bind_param("ss", $gatewayId, $macAddress);

if (!$stmt->execute())
{
    error_log("Provision query failed: " . $stmt->error);
    jsonError(500, "Provisioning database error");
}

$result = $stmt->get_result();
if ($result->num_rows !== 1)
{
    rejectProvisioning();
}

$gateway = $result->fetch_assoc();
if (empty($gateway["bootstrap_secret_hash"]) ||
    !password_verify($bootstrapSecret, $gateway["bootstrap_secret_hash"]))
{
    rejectProvisioning();
}

if (empty($gateway["api_key"]))
{
    error_log("Gateway has no API key: " . $gatewayId);
    jsonError(500, "Gateway configuration is incomplete");
}

// ---------------------------------------
// Build gateway.json. Wi-Fi stays in NVS.
// ---------------------------------------
$configuration = [
    "gateway" => [
        "gatewayId" => $gateway["gateway_id"],
        "firmware" => $firmware,
        "apiKey" => $gateway["api_key"],
        "hardware" => $hardware,
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

// ---------------------------------------
// Update gateway provisioning status
// ---------------------------------------
$stmt = $conn->prepare(
"
UPDATE gateways
SET
    firmware = ?,
    hardware = ?,
    wifi_networks = ?,
    provisioned_at = COALESCE(provisioned_at, NOW()),
    last_seen = NOW()
WHERE gateway_id = ?
"
);

if (!$stmt)
{
    error_log("Provision update prepare failed: " . $conn->error);
    jsonError(500, "Provisioning database error");
}

$stmt->bind_param("ssss", $firmware, $hardware, $wifiNetworksJson, $gatewayId);
if (!$stmt->execute())
{
    error_log("Provision update failed: " . $stmt->error);
    jsonError(500, "Provisioning database error");
}

// ---------------------------------------
// Success response expected by ESP32
// ---------------------------------------
jsonResponse(200, [
    "status" => "OK",
    "success" => true,
    "message" => "Gateway provisioned",
    "gatewayId" => $gatewayId,
    "configurationVersion" => (int)$gateway["config_version"],
    "configuration" => $configuration
]);

?>

<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_REQUEST_BYTES = 4096;
const MAX_FIRMWARE_BYTES = 3264 * 1024;
const FIRMWARE_BASE_URL = "https://www.geworks.co.in/energy/firmware/";

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
    error_log("FIRMWARE MANIFEST UNHANDLED EXCEPTION: " . $e->getMessage());
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
    fail(400, "Invalid manifest request");

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

$stmt = $conn->prepare(
    "SELECT parameters FROM gateway_commands
     WHERE id = ? AND gateway_id = ? AND command = 'UPDATE_FIRMWARE'
       AND status IN ('DELIVERED', 'RUNNING')
       AND (expires_at IS NULL OR expires_at > NOW())
     LIMIT 1"
);
if (!$stmt)
    fail(500, "Database error");
$stmt->bind_param("is", $commandId, $gatewayId);
if (!$stmt->execute())
    fail(500, "Database error");
$result = $stmt->get_result();
if ($result->num_rows !== 1)
    fail(404, "Firmware command not available");
$row = $result->fetch_assoc();
$stmt->close();

$parameters = json_decode((string)$row["parameters"], true);
if (!is_array($parameters))
    fail(500, "Firmware command parameters are invalid");
$version = isset($parameters["version"]) && is_string($parameters["version"])
    ? trim($parameters["version"]) : "";
$filename = isset($parameters["filename"]) && is_string($parameters["filename"])
    ? trim($parameters["filename"]) : "";
$sha256 = isset($parameters["sha256"]) && is_string($parameters["sha256"])
    ? strtolower(trim($parameters["sha256"])) : "";
$size = isset($parameters["size"]) && is_numeric($parameters["size"])
    ? (int)$parameters["size"] : 0;

if ($version === "" || strlen($version) > 100 ||
    !preg_match('/^[A-Za-z0-9._-]+\.bin$/', $filename) ||
    !preg_match('/^[a-f0-9]{64}$/', $sha256) ||
    $size <= 0 || $size > MAX_FIRMWARE_BYTES)
{
    fail(500, "Firmware command parameters are invalid");
}

$localFile = dirname(__DIR__) . DIRECTORY_SEPARATOR . "firmware" .
             DIRECTORY_SEPARATOR . $filename;
if (!is_file($localFile) || filesize($localFile) !== $size)
    fail(404, "Firmware file is missing or has a different size");

respond(200, [
    "status" => "OK",
    "success" => true,
    "commandId" => $commandId,
    "version" => $version,
    "url" => FIRMWARE_BASE_URL . rawurlencode($filename),
    "size" => $size,
    "sha256" => $sha256
]);


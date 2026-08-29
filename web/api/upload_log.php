<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
header("X-Content-Type-Options: nosniff");

require_once "config.php";
mysqli_report(MYSQLI_REPORT_OFF);

const MAX_LOG_BYTES = 600 * 1024;

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

set_exception_handler(function (Throwable $e): void
{
    error_log("LOG UPLOAD UNHANDLED EXCEPTION: " . $e->getMessage());
    fail(500, "Internal server error");
});

if ($_SERVER["REQUEST_METHOD"] !== "POST")
{
    header("Allow: POST");
    fail(405, "Only POST allowed");
}

$remoteName = isset($_GET["filename"]) && is_string($_GET["filename"])
    ? trim($_GET["filename"])
    : "";
if (!in_array($remoteName, ["current.log", "previous.log"], true))
    fail(400, "Invalid log filename");

$gatewayId = trim((string)($_SERVER["HTTP_X_GATEWAY_ID"] ?? ""));
$apiKey = (string)($_SERVER["HTTP_X_API_KEY"] ?? "");
if (!preg_match('/^GEW[0-9]{6,}$/', $gatewayId) || $apiKey === "" || strlen($apiKey) > 255)
    fail(401, "Invalid gateway credentials");

$stmt = $conn->prepare(
    "SELECT gateway_id FROM gateways
     WHERE gateway_id = ? AND api_key = ? LIMIT 1"
);
if (!$stmt)
{
    error_log("Log authentication prepare failed: " . $conn->error);
    fail(500, "Database error");
}
$stmt->bind_param("ss", $gatewayId, $apiKey);
if (!$stmt->execute())
{
    error_log("Log authentication query failed: " . $stmt->error);
    fail(500, "Database error");
}
if ($stmt->get_result()->num_rows !== 1)
    fail(401, "Invalid gateway credentials");
$stmt->close();

$contentLength = (int)($_SERVER["CONTENT_LENGTH"] ?? 0);
if ($contentLength <= 0 || $contentLength > MAX_LOG_BYTES)
    fail(413, "Invalid log size");

$logData = file_get_contents("php://input", false, null, 0, MAX_LOG_BYTES + 1);
if ($logData === false || strlen($logData) === 0 || strlen($logData) > MAX_LOG_BYTES)
    fail(413, "Invalid log size");

$logDirectory = __DIR__ . DIRECTORY_SEPARATOR . "logs";
if (!is_dir($logDirectory) &&
    !mkdir($logDirectory, 0750, true) &&
    !is_dir($logDirectory))
{
    error_log("Unable to create log directory: " . $logDirectory);
    fail(500, "Unable to create log directory");
}

$suffix = $remoteName === "current.log" ? "current.log" : "previous.log";
$destination = $logDirectory . DIRECTORY_SEPARATOR . $gatewayId . "_" . $suffix;
$temporary = $destination . ".tmp";

if (file_put_contents($temporary, $logData, LOCK_EX) !== strlen($logData))
{
    @unlink($temporary);
    fail(500, "Unable to save log file");
}

if (!rename($temporary, $destination))
{
    @unlink($temporary);
    fail(500, "Unable to activate log file");
}

respond(200, [
    "status" => "OK",
    "message" => "Log uploaded",
    "gatewayId" => $gatewayId,
    "filename" => basename($destination),
    "bytes" => strlen($logData)
]);


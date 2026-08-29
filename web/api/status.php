<?php
declare(strict_types=1);

header("Content-Type: application/json; charset=utf-8");
header("Cache-Control: no-store");
require_once __DIR__ . "/config.php";
require_once __DIR__ . "/../includes/auth.php";

$user = requireApiUser();
$gatewayId = trim((string)($_GET["gatewayId"] ?? ""));
if ($gatewayId === "" || !preg_match('/^[A-Za-z0-9_-]{1,64}$/', $gatewayId)) {
    http_response_code(400);
    echo json_encode(["status" => "ERROR", "message" => "Valid gatewayId required"]);
    exit;
}
requireGatewayAccess($conn, (int)$user["id"], $gatewayId);

$statement = $conn->prepare(
    "SELECT g.gateway_id, g.firmware, g.last_seen, g.last_sequence,
            r.sequence, r.reading_time, r.firmware AS reading_firmware
     FROM gateways g
     LEFT JOIN meter_readings r ON r.id = (
         SELECT mr.id FROM meter_readings mr WHERE mr.gateway_id = g.gateway_id
         ORDER BY mr.id DESC LIMIT 1
     )
     WHERE g.gateway_id = ? LIMIT 1"
);
$statement->bind_param("s", $gatewayId);
$statement->execute();
$row = $statement->get_result()->fetch_assoc();
$statement->close();

$gateways = [];
if ($row) {
    $seenAt = $row["reading_time"] ?: $row["last_seen"];
    $seenTimestamp = $seenAt ? strtotime($seenAt) : false;
    $gateways[] = [
        "gatewayId" => $row["gateway_id"],
        "status" => ($seenTimestamp !== false && time() - $seenTimestamp <= 600) ? "ONLINE" : "OFFLINE",
        "lastSeen" => $seenAt,
        "lastSequence" => $row["sequence"] ?? $row["last_sequence"],
        "firmware" => $row["reading_firmware"] ?: $row["firmware"],
    ];
}

echo json_encode([
    "status" => "OK",
    "serverTime" => date("Y-m-d H:i:s"),
    "gateways" => $gateways,
], JSON_UNESCAPED_SLASHES);
?>

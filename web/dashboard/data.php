<?php
declare(strict_types=1);

require_once __DIR__ . "/../api/config.php";
require_once __DIR__ . "/../includes/auth.php";

$dashboardUser = requireDashboardUser();
$userId = (int)$dashboardUser["id"];
$statement = $conn->prepare(
    "SELECT g.gateway_id, g.firmware, g.last_seen, g.last_sequence,
            r.reading_time, r.sequence, r.firmware AS reading_firmware
     FROM user_gateway_access uga
     INNER JOIN gateways g ON g.gateway_id = uga.gateway_id
     LEFT JOIN meter_readings r ON r.id = (
         SELECT mr.id FROM meter_readings mr WHERE mr.gateway_id = g.gateway_id
         ORDER BY mr.id DESC LIMIT 1
     )
     WHERE uga.user_id = ? ORDER BY g.gateway_id"
);
$statement->bind_param("i", $userId);
$statement->execute();
$result = $statement->get_result();
$authorizedGateways = [];
while ($row = $result->fetch_assoc()) {
    $seenAt = $row["reading_time"] ?: $row["last_seen"];
    $seenTimestamp = $seenAt ? strtotime($seenAt) : false;
    $authorizedGateways[] = [
        "gatewayId" => $row["gateway_id"],
        "status" => ($seenTimestamp !== false && time() - $seenTimestamp <= 600) ? "ONLINE" : "OFFLINE",
        "lastSeen" => $seenAt,
        "lastSequence" => $row["sequence"] ?? $row["last_sequence"],
        "firmware" => $row["reading_firmware"] ?: $row["firmware"],
    ];
}
$statement->close();

$requestedGatewayId = trim((string)($_GET["gatewayId"] ?? ""));
$gateway = null;
foreach ($authorizedGateways as $candidate) {
    if ($requestedGatewayId !== "" && hash_equals($candidate["gatewayId"], $requestedGatewayId)) {
        $gateway = $candidate;
        break;
    }
}
if ($gateway === null && $authorizedGateways !== []) {
    $gateway = $authorizedGateways[0];
}

$gatewayId = $gateway["gatewayId"] ?? "";
$readings = [];
if ($gatewayId !== "") {
    $statement = $conn->prepare(
        "SELECT sequence, reading_time, firmware, data FROM meter_readings
         WHERE gateway_id = ? ORDER BY id DESC LIMIT 120"
    );
    $statement->bind_param("s", $gatewayId);
    $statement->execute();
    $result = $statement->get_result();
    while ($row = $result->fetch_assoc()) {
        $measurements = json_decode($row["data"], true);
        $readings[] = [
            "sequence" => (int)$row["sequence"],
            "time" => $row["reading_time"],
            "firmware" => $row["firmware"],
            "measurements" => is_array($measurements) ? $measurements : [],
        ];
    }
    $statement->close();
}

$latest = $readings[0] ?? null;
$jsReadings = json_encode($readings, JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE |
    JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_APOS | JSON_HEX_QUOT);
?>

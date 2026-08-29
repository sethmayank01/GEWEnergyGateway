<?php

header("Content-Type: application/json");

require_once "config.php";
require_once __DIR__ . "/../includes/auth.php";

$apiUser = requireApiUser();


// ---------------------------------------
// Input Parameters
// ---------------------------------------

$gatewayId = $_GET["gatewayId"] ?? null;

$limit = isset($_GET["limit"])
    ? intval($_GET["limit"])
    : 100;


// Safety limit

$limit = max(1, min($limit, 1000));



// ---------------------------------------
// Validate Gateway
// ---------------------------------------

if($gatewayId == null || !preg_match('/^[A-Za-z0-9_-]{1,64}$/', $gatewayId))
{
    http_response_code(400);

    echo json_encode([
        "status"=>"ERROR",
        "message"=>"Valid gatewayId required"
    ]);

    exit;
}

requireGatewayAccess($conn, (int)$apiUser["id"], $gatewayId);



// ---------------------------------------
// Fetch Data
// ---------------------------------------

$sql = "
SELECT

id,
gateway_id,
sequence,
reading_time,
firmware,
data

FROM meter_readings

WHERE gateway_id = ?

ORDER BY id DESC

LIMIT ?
";



$stmt = $conn->prepare($sql);


$stmt->bind_param(
    "si",
    $gatewayId,
    $limit
);


$stmt->execute();


$result = $stmt->get_result();



// ---------------------------------------
// Prepare Response
// ---------------------------------------

$readings = [];


while($row = $result->fetch_assoc())
{

    $readings[] = [

        "sequence" =>
            intval($row["sequence"]),


        "time" =>
            $row["reading_time"],


        "firmware" =>
            $row["firmware"],


        "measurements" =>
            json_decode(
                $row["data"],
                true
            )

    ];

}



// ---------------------------------------
// Response
// ---------------------------------------

echo json_encode(

[
    "status"=>"OK",

    "gatewayId"=>$gatewayId,

    "count"=>count($readings),

    "readings"=>$readings
],

JSON_PRETTY_PRINT

);


?>

<?php

header("Content-Type: application/json; charset=utf-8");

require_once "config.php";
require_once __DIR__ . "/../includes/auth.php";

$apiUser = requireApiUser();


// ============================================================
// INPUT PARAMETERS
// ============================================================

$gatewayId = $_GET["gatewayId"] ?? "";

$range = $_GET["range"] ?? "1h";


// ============================================================
// VALIDATE GATEWAY
// ============================================================

if ($gatewayId === "" || !preg_match('/^[A-Za-z0-9_-]{1,64}$/', $gatewayId))
{
    http_response_code(400);

    echo json_encode([
        "status" => "ERROR",
        "message" => "Valid gatewayId required"
    ]);

    exit;
}

requireGatewayAccess($conn, (int)$apiUser["id"], $gatewayId);


// ============================================================
// DETERMINE TIME RANGE
// ============================================================

$now = new DateTime();

$from = null;
$to   = clone $now;


switch ($range)
{

    case "1h":

        $from = clone $now;
        $from->modify("-1 hour");

        break;


    case "6h":

        $from = clone $now;
        $from->modify("-6 hours");

        break;


    case "12h":

        $from = clone $now;
        $from->modify("-12 hours");

        break;


    case "24h":

        $from = clone $now;
        $from->modify("-24 hours");

        break;


    case "7d":

        $from = clone $now;
        $from->modify("-7 days");

        break;


    case "custom":

        $fromInput = $_GET["from"] ?? "";
        $toInput   = $_GET["to"] ?? "";


        if (
            $fromInput === "" ||
            $toInput === ""
        )
        {
            http_response_code(400);

            echo json_encode([
                "status" => "ERROR",
                "message" =>
                    "from and to are required"
            ]);

            exit;
        }


        try
        {
            $from = new DateTime($fromInput);
            $to   = new DateTime($toInput);
        }
        catch(Exception $e)
        {
            http_response_code(400);

            echo json_encode([
                "status" => "ERROR",
                "message" => "Invalid date/time"
            ]);

            exit;
        }

        break;


    default:

        http_response_code(400);

        echo json_encode([
            "status" => "ERROR",
            "message" => "Invalid range"
        ]);

        exit;
}


// ============================================================
// VALIDATE DATE RANGE
// ============================================================

if ($from >= $to)
{
    http_response_code(400);

    echo json_encode([
        "status" => "ERROR",
        "message" => "Invalid date range"
    ]);

    exit;
}


$fromSql =
    $from->format("Y-m-d H:i:s");

$toSql =
    $to->format("Y-m-d H:i:s");


// ============================================================
// FETCH EVERY READING
// ============================================================
//
// IMPORTANT:
// No LIMIT here.
//
// For 7 days we want EVERY reading.
// For example, if gateway records every 30 seconds:
//
// 1 hour  ≈ 120 readings
// 6 hours ≈ 720 readings
// 24 hours ≈ 2,880 readings
// 7 days  ≈ 20,160 readings
//
// No averaging / scattering is done here.
// The browser receives the actual readings.
// ============================================================

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

  AND reading_time >= ?

  AND reading_time <= ?

ORDER BY reading_time ASC,
         id ASC

";


$stmt = $conn->prepare($sql);


if (!$stmt)
{
    http_response_code(500);

    echo json_encode([
        "status" => "ERROR",
        "message" => "SQL prepare failed"
    ]);

    exit;
}


$stmt->bind_param(
    "sss",
    $gatewayId,
    $fromSql,
    $toSql
);


if (!$stmt->execute())
{
    http_response_code(500);

    echo json_encode([
        "status" => "ERROR",
        "message" => "SQL execution failed"
    ]);

    exit;
}


$result =
    $stmt->get_result();


// ============================================================
// BUILD READINGS ARRAY
// ============================================================

$readings = [];


while ($row = $result->fetch_assoc())
{

    $measurements =
        json_decode(
            $row["data"],
            true
        );


    if (!is_array($measurements))
    {
        continue;
    }


    /*
     * Your current readings.php stores the decoded
     * data directly under "measurements".
     *
     * If data ever contains the complete gateway packet
     * with a nested "measurements" object, handle that too.
     */

    if (
        isset($measurements["measurements"]) &&
        is_array($measurements["measurements"])
    )
    {
        $measurements =
            $measurements["measurements"];
    }


    $readings[] = [

        "sequence" =>
            intval($row["sequence"]),

        "time" =>
            $row["reading_time"],

        "firmware" =>
            $row["firmware"],

        "measurements" =>
            $measurements

    ];
}


// ============================================================
// RESPONSE
// ============================================================

echo json_encode(

    [
        "status" =>
            "OK",

        "gatewayId" =>
            $gatewayId,

        "range" =>
            $range,

        "from" =>
            $fromSql,

        "to" =>
            $toSql,

        "count" =>
            count($readings),

        "readings" =>
            $readings
    ],

    JSON_UNESCAPED_SLASHES |
    JSON_UNESCAPED_UNICODE

);


$stmt->close();

$conn->close();

?>

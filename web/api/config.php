<?php

$host = "localhost";
$db   = "geworksc_energymanagement";
$user = "geworksc_energymanagement";
$pass = "Gew@1973";

$conn = new mysqli($host, $user, $pass, $db);

if ($conn->connect_error)
{
    http_response_code(500);

    die(json_encode([
        "status"=>"ERROR",
        "message"=>"Database connection failed"
    ]));
}
$conn->set_charset("utf8mb4");
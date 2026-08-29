<?php
declare(strict_types=1);
require_once __DIR__ . "/includes/auth.php";
if ($_SERVER["REQUEST_METHOD"] !== "POST" ||
    !validCsrfToken((string)($_POST["csrf_token"] ?? "")))
{
    http_response_code(400);
    exit("Invalid logout request");
}
logoutUser();
header("Location: /energy/login.php");
exit;


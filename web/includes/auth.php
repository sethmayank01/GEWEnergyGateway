<?php
declare(strict_types=1);

const GEW_SESSION_IDLE_SECONDS = 1800;
const GEW_SESSION_MAX_SECONDS = 28800;

function startGewSession(): void
{
    if (session_status() === PHP_SESSION_ACTIVE)
        return;

    session_name("GEWSESSID");
    session_set_cookie_params([
        "lifetime" => 0,
        "path" => "/energy/",
        "secure" => !empty($_SERVER["HTTPS"]) && $_SERVER["HTTPS"] !== "off",
        "httponly" => true,
        "samesite" => "Lax"
    ]);
    session_start();
}

function currentUser(): ?array
{
    startGewSession();
    if (empty($_SESSION["user_id"]) || empty($_SESSION["username"]))
        return null;

    $now = time();
    $created = (int)($_SESSION["created_at"] ?? 0);
    $lastActivity = (int)($_SESSION["last_activity"] ?? 0);
    if ($created <= 0 || $lastActivity <= 0 ||
        $now - $created > GEW_SESSION_MAX_SECONDS ||
        $now - $lastActivity > GEW_SESSION_IDLE_SECONDS)
    {
        logoutUser();
        return null;
    }

    $_SESSION["last_activity"] = $now;
    return [
        "id" => (int)$_SESSION["user_id"],
        "username" => (string)$_SESSION["username"],
        "displayName" => (string)($_SESSION["display_name"] ?? $_SESSION["username"])
    ];
}

function loginUser(array $row): void
{
    startGewSession();
    session_regenerate_id(true);
    $_SESSION["user_id"] = (int)$row["id"];
    $_SESSION["username"] = (string)$row["username"];
    $_SESSION["display_name"] = (string)($row["display_name"] ?: $row["username"]);
    $_SESSION["created_at"] = time();
    $_SESSION["last_activity"] = time();
    $_SESSION["csrf_token"] = bin2hex(random_bytes(32));
}

function logoutUser(): void
{
    startGewSession();
    $_SESSION = [];
    if (ini_get("session.use_cookies"))
    {
        $parameters = session_get_cookie_params();
        setcookie(session_name(), "", time() - 42000,
            $parameters["path"], $parameters["domain"],
            (bool)$parameters["secure"], (bool)$parameters["httponly"]);
    }
    session_destroy();
}

function requireDashboardUser(): array
{
    $user = currentUser();
    if ($user === null)
    {
        $next = rawurlencode($_SERVER["REQUEST_URI"] ?? "/energy/dashboard.php");
        header("Location: /energy/login.php?next=" . $next);
        exit;
    }
    return $user;
}

function requireApiUser(): array
{
    $user = currentUser();
    if ($user === null)
    {
        http_response_code(401);
        echo json_encode(["status" => "ERROR", "message" => "Authentication required"]);
        exit;
    }
    return $user;
}

function userCanAccessGateway(mysqli $conn, int $userId, string $gatewayId): bool
{
    $statement = $conn->prepare(
        "SELECT 1 FROM user_gateway_access
         WHERE user_id = ? AND gateway_id = ? LIMIT 1"
    );
    if (!$statement)
        return false;
    $statement->bind_param("is", $userId, $gatewayId);
    $statement->execute();
    $allowed = $statement->get_result()->num_rows === 1;
    $statement->close();
    return $allowed;
}

function requireGatewayAccess(mysqli $conn, int $userId, string $gatewayId): void
{
    if (!userCanAccessGateway($conn, $userId, $gatewayId))
    {
        http_response_code(403);
        echo json_encode(["status" => "ERROR", "message" => "Gateway access denied"]);
        exit;
    }
}

function csrfToken(): string
{
    startGewSession();
    if (empty($_SESSION["csrf_token"]))
        $_SESSION["csrf_token"] = bin2hex(random_bytes(32));
    return (string)$_SESSION["csrf_token"];
}

function validCsrfToken(string $token): bool
{
    startGewSession();
    return !empty($_SESSION["csrf_token"]) &&
           hash_equals((string)$_SESSION["csrf_token"], $token);
}


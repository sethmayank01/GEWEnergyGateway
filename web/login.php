<?php
declare(strict_types=1);

require_once __DIR__ . "/api/config.php";
require_once __DIR__ . "/includes/auth.php";

if (currentUser() !== null)
{
    header("Location: /energy/dashboard.php");
    exit;
}

$error = "";
$next = isset($_GET["next"]) && is_string($_GET["next"])
    ? $_GET["next"] : "/energy/dashboard.php";
if (!str_starts_with($next, "/energy/") || str_starts_with($next, "//"))
    $next = "/energy/dashboard.php";

if ($_SERVER["REQUEST_METHOD"] === "POST")
{
    $username = strtolower(trim((string)($_POST["username"] ?? "")));
    $password = (string)($_POST["password"] ?? "");
    $token = (string)($_POST["csrf_token"] ?? "");
    $next = (string)($_POST["next"] ?? "/energy/dashboard.php");

    if (!validCsrfToken($token))
        $error = "The login form expired. Please try again.";
    elseif (!preg_match('/^[a-z0-9._@-]{3,100}$/', $username) || strlen($password) > 200)
        $error = "Invalid username or password.";
    else
    {
        $statement = $conn->prepare(
            "SELECT id, username, display_name, password_hash
             FROM dashboard_users
             WHERE username = ? AND status = 'ACTIVE' LIMIT 1"
        );
        $statement->bind_param("s", $username);
        $statement->execute();
        $row = $statement->get_result()->fetch_assoc();
        $statement->close();

        if ($row && password_verify($password, (string)$row["password_hash"]))
        {
            loginUser($row);
            $update = $conn->prepare("UPDATE dashboard_users SET last_login = NOW() WHERE id = ?");
            $update->bind_param("i", $row["id"]);
            $update->execute();
            $update->close();
            if (!str_starts_with($next, "/energy/") || str_starts_with($next, "//"))
                $next = "/energy/dashboard.php";
            header("Location: " . $next);
            exit;
        }
        $error = "Invalid username or password.";
    }
}
?>
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>GEW Energy Login</title><link rel="stylesheet" href="assets/login.css"></head>
<body><main class="login-card"><h1>GEW Energy</h1><p>Sign in to view your gateways.</p>
<?php if ($error !== ""): ?><div class="login-error"><?= htmlspecialchars($error) ?></div><?php endif; ?>
<form method="post" autocomplete="on">
<input type="hidden" name="csrf_token" value="<?= htmlspecialchars(csrfToken()) ?>">
<input type="hidden" name="next" value="<?= htmlspecialchars($next) ?>">
<label>Username<input name="username" maxlength="100" autocomplete="username" required autofocus></label>
<label>Password<input type="password" name="password" maxlength="200" autocomplete="current-password" required></label>
<button type="submit">Sign in</button></form></main></body></html>


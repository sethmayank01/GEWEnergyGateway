<?php

require_once __DIR__ . "/dashboard/data.php";
header("Cache-Control: private, no-store");

?>
<!DOCTYPE html>
<html>

<head>

<meta charset="utf-8">

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>GEW Energy Monitoring</title>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<link rel="stylesheet"
      href="assets/dashboard.css">

</head>

<body>

<?php include __DIR__ . "/dashboard/header.php"; ?>

<?php include __DIR__ . "/dashboard/tabs.php"; ?>

<div class="container">

<?php include __DIR__ . "/dashboard/live.php"; ?>

<?php include __DIR__ . "/dashboard/trends.php"; ?>

<?php include __DIR__ . "/dashboard/energy.php"; ?>

<?php include __DIR__ . "/dashboard/quality.php"; ?>

<?php include __DIR__ . "/dashboard/events.php"; ?>

</div>

<?php include __DIR__ . "/dashboard/footer.php"; ?>

<script>
/*
 * Initial data is rendered once by PHP.
 * dashboard.js subsequently refreshes the APIs every 30 seconds
 * without reloading the page or changing the selected tab.
 */
window.GEW_DASHBOARD_READINGS =
    <?= $jsReadings ?: "[]" ?>;

window.GEW_GATEWAY_ID =
    <?= json_encode($gatewayId) ?>;
</script>

<script src="assets/dashboard.js?v=4"></script>

</body>
</html>

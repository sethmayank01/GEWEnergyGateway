<header class="header">
<div class="header-left">
    <h1>⚡ GEW Energy Monitoring</h1>
    <div class="gateway-line">
        <?php if ($authorizedGateways !== []): ?>
            <form method="get" action="" class="gateway-selector">
                <label for="gatewayId">Gateway:</label>
                <select id="gatewayId" name="gatewayId" onchange="this.form.submit()">
                    <?php foreach ($authorizedGateways as $item): ?>
                        <option value="<?= htmlspecialchars($item["gatewayId"]) ?>"
                            <?= $item["gatewayId"] === $gatewayId ? "selected" : "" ?>>
                            <?= htmlspecialchars($item["gatewayId"]) ?>
                        </option>
                    <?php endforeach; ?>
                </select>
            </form>
        <?php else: ?>
            <span>No gateway has been assigned to this account.</span>
        <?php endif; ?>
        <span>Last Data Update: <span id="pageRefreshTime">-</span></span>
    </div>
</div>

<div class="gateway-status">
    <div class="account-actions">
        <span><?= htmlspecialchars($dashboardUser["displayName"]) ?></span>
        <form method="post" action="/energy/logout.php">
            <input type="hidden" name="csrf_token" value="<?= htmlspecialchars(csrfToken()) ?>">
            <button type="submit">Sign out</button>
        </form>
    </div>
    <?php if ($gateway): ?>
        <?php $online = strtolower($gateway["status"] ?? "") === "online"; ?>
        <span id="pageGatewayStatus" class="<?= $online ? "status-online" : "status-offline" ?>">
            ● <?= htmlspecialchars($gateway["status"] ?? "UNKNOWN") ?>
        </span><br>
        <span class="last-seen-label">Last Seen:
            <span id="pageLastSeen"><?= htmlspecialchars($latest["time"] ?? $gateway["lastSeen"] ?? "-") ?></span>
        </span>
    <?php else: ?>
        <span id="pageGatewayStatus" class="status-offline">● No Gateway Assigned</span><br>
        <span class="last-seen-label">Last Seen: <span id="pageLastSeen">-</span></span>
    <?php endif; ?>
</div>
</header>

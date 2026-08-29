<div id="events"
     class="tab-content">


<div class="section">

<div class="section-title">
Gateway Information
</div>


<div class="card">

<table class="info-table">


<tr>

<td>Gateway ID</td>

<td>
<?=htmlspecialchars($gatewayId)?>
</td>

</tr>


<tr>

<td>Status</td>

<td>

<?php if($gateway): ?>

<span class="<?=$online ?
    "status-online" :
    "status-offline"?>">

●
<?=htmlspecialchars(
    $gateway["status"] ?? "-"
)?>

</span>

<?php else: ?>

-

<?php endif; ?>

</td>

</tr>


<tr>

<td>Firmware</td>

<td>
<?=htmlspecialchars(
    $gateway["firmware"] ?? "-"
)?>
</td>

</tr>


<tr>

<td>Last Seen</td>

<td>
<?=htmlspecialchars(
    $gateway["lastSeen"] ?? "-"
)?>
</td>

</tr>


<tr>

<td>Latest Sequence</td>

<td>
<?=htmlspecialchars(
    $latest["sequence"] ?? "-"
)?>
</td>

</tr>


<tr>

<td>Readings Available</td>

<td>
<?=count($readings)?>
</td>

</tr>


<tr>

<td>Dashboard Update</td>

<td>
30 seconds
</td>

</tr>


</table>

</div>

</div>


<div class="section">

<div class="section-title">
Prototype Monitoring
</div>


<div class="card">

<p style="color:#94a3b8;margin:0;line-height:1.6;">

The gateway is currently operating with a
30-second measurement/upload interval.

The dashboard analyses the latest 120 readings,
representing approximately one hour of operation.

Detailed gateway events and alarm history can be
added later without changing the live monitoring
screen.

</p>

</div>

</div>


</div>


</div>

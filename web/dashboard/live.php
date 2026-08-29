<div id="live"
     class="tab-content active">

<?php if($latest): ?>



<!-- ========================================================
     ELECTRICAL SUMMARY
     ======================================================== -->

<div class="live-section-title">
    Electrical Summary
</div>


<div class="live-summary-grid">


    <!-- L-N Average -->

    <div class="live-summary-card">

        <div class="summary-label">
            L-N AVERAGE
        </div>

        <div class="summary-value">

            <span id="liveVNavg">
                <?=number_format(
                    $latest["measurements"]["voltage"]["lnAverage"] ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                V
            </span>

        </div>

    </div>


    <!-- L-L Average -->

    <div class="live-summary-card">

        <div class="summary-label">
            L-L AVERAGE
        </div>

        <div class="summary-value">

            <span id="liveVLLavg">
                <?=number_format(
                    $latest["measurements"]["voltage"]["llAverage"] ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                V
            </span>

        </div>

    </div>


    <!-- Average Current -->

    <div class="live-summary-card">

        <div class="summary-label">
            AVG CURRENT
        </div>

        <div class="summary-value">

            <span id="liveIAvg">
                <?=number_format(
                    $latest["measurements"]["current"]["average"]
                    ?? $latest["measurements"]["current"]["lAverage"]
                    ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                A
            </span>

        </div>

    </div>


    <!-- Active Power -->

    <div class="live-summary-card power-highlight">

        <div class="summary-label">
            ACTIVE POWER
        </div>

        <div class="summary-value">

            <span id="liveKW">
                <?=number_format(
                    $latest["measurements"]["power"]["active"] ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                kW
            </span>

        </div>

    </div>


    <!-- Power Factor -->

    <div class="live-summary-card">

        <div class="summary-label">
            POWER FACTOR
        </div>

        <div class="summary-value">

            <span id="livePF">
                <?=number_format(
                    $latest["measurements"]["powerFactor"]["average"] ?? 0,
                    3
                )?>
            </span>

        </div>

        <div class="summary-sub"
             id="livePFType">

            <?=
                (
                    ($latest["measurements"]["powerFactor"]["average"] ?? 0)
                    < 0
                )
                ? "CAPACITIVE"
                : "INDUCTIVE"
            ?>

        </div>

    </div>


    <!-- Apparent Power -->

    <div class="live-summary-card">

        <div class="summary-label">
            APPARENT POWER
        </div>

        <div class="summary-value">

            <span id="liveKVA">
                <?=number_format(
                    $latest["measurements"]["power"]["apparent"] ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                kVA
            </span>

        </div>

    </div>


    <!-- Reactive Power -->

    <div class="live-summary-card">

        <div class="summary-label">
            REACTIVE POWER
        </div>

        <div class="summary-value">

            <span id="liveKVAR">
                <?=number_format(
                    $latest["measurements"]["power"]["reactive"] ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                kVAr
            </span>

        </div>

    </div>


    <!-- Frequency -->

    <div class="live-summary-card">

        <div class="summary-label">
            FREQUENCY
        </div>

        <div class="summary-value">

            <span id="liveFreq">
                <?=number_format(
                    $latest["measurements"]["frequency"] ?? 0,
                    2
                )?>
            </span>

            <span class="summary-unit">
                Hz
            </span>

        </div>

    </div>


</div>


<!-- ========================================================
     VOLTAGE
     ======================================================== -->

<div class="live-section-title">
    Voltage
</div>


<div class="live-panel">


    <div class="measurement-group-title">
        Line-to-Neutral
    </div>


    <div class="measurement-grid four">


        <div class="measurement-card">

            <div class="measurement-label">
                L1
            </div>

            <div class="measurement-value">

                <span id="liveV1">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["l1"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L2
            </div>

            <div class="measurement-value">

                <span id="liveV2">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["l2"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L3
            </div>

            <div class="measurement-value">

                <span id="liveV3">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["l3"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


        <div class="measurement-card average-card">

            <div class="measurement-label">
                AVERAGE
            </div>

            <div class="measurement-value">

                <span id="liveVNavg2">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["lnAverage"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


    </div>


    <div class="measurement-group-title">
        Line-to-Line
    </div>


    <div class="measurement-grid four">


        <div class="measurement-card">

            <div class="measurement-label">
                L12
            </div>

            <div class="measurement-value">

                <span id="liveV12">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["l12"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L23
            </div>

            <div class="measurement-value">

                <span id="liveV23">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["l23"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L31
            </div>

            <div class="measurement-value">

                <span id="liveV31">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["l31"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


        <div class="measurement-card average-card">

            <div class="measurement-label">
                AVERAGE
            </div>

            <div class="measurement-value">

                <span id="liveVLLavg2">
                    <?=number_format(
                        $latest["measurements"]["voltage"]["llAverage"] ?? 0,
                        2
                    )?>
                </span>

                <span>V</span>

            </div>

        </div>


    </div>

</div>


<!-- ========================================================
     CURRENT
     ======================================================== -->

<div class="live-section-title">
    Current
</div>


<div class="live-panel">


    <div class="measurement-grid four">


        <div class="measurement-card">

            <div class="measurement-label">
                L1
            </div>

            <div class="measurement-value">

                <span id="liveI1">
                    <?=number_format(
                        $latest["measurements"]["current"]["l1"] ?? 0,
                        2
                    )?>
                </span>

                <span>A</span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L2
            </div>

            <div class="measurement-value">

                <span id="liveI2">
                    <?=number_format(
                        $latest["measurements"]["current"]["l2"] ?? 0,
                        2
                    )?>
                </span>

                <span>A</span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L3
            </div>

            <div class="measurement-value">

                <span id="liveI3">
                    <?=number_format(
                        $latest["measurements"]["current"]["l3"] ?? 0,
                        2
                    )?>
                </span>

                <span>A</span>

            </div>

        </div>


        <div class="measurement-card average-card">

            <div class="measurement-label">
                AVERAGE
            </div>

            <div class="measurement-value">

                <span id="liveIAvg2">
                    <?=number_format(
                        $latest["measurements"]["current"]["average"]
                        ?? $latest["measurements"]["current"]["lAverage"]
                        ?? 0,
                        2
                    )?>
                </span>

                <span>A</span>

            </div>

        </div>


    </div>

</div>


<!-- ========================================================
     POWER FACTOR
     ======================================================== -->

<div class="live-section-title">
    Power Factor
</div>


<div class="live-panel">


    <div class="measurement-grid four">


        <div class="measurement-card average-card">

            <div class="measurement-label">
                AVERAGE
            </div>

            <div class="measurement-value">

                <span id="livePF2" class="pf-value">
                    <?=number_format(
                        $latest["measurements"]["powerFactor"]["average"] ?? 0,
                        3
                    )?>
                </span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L1
            </div>

            <div class="measurement-value">

                <span id="livePFL1" class="pf-value">
                    <?=number_format(
                        $latest["measurements"]["powerFactor"]["l1"] ?? 0,
                        3
                    )?>
                </span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L2
            </div>

            <div class="measurement-value">

                <span id="livePFL2" class="pf-value">
                    <?=number_format(
                        $latest["measurements"]["powerFactor"]["l2"] ?? 0,
                        3
                    )?>
                </span>

            </div>

        </div>


        <div class="measurement-card">

            <div class="measurement-label">
                L3
            </div>

            <div class="measurement-value">

                <span id="livePFL3" class="pf-value">
                    <?=number_format(
                        $latest["measurements"]["powerFactor"]["l3"] ?? 0,
                        3
                    )?>
                </span>

            </div>

        </div>


    </div>

</div>


<!-- ========================================================
     SYSTEM
     ======================================================== -->

<div class="live-system-row">


    <div class="live-system-card">

        <div class="system-label">
            ENERGY RECEIVED
        </div>

        <div class="system-value">

            <span id="liveEnergy">
                <?=number_format(
                    $latest["measurements"]["energyReceivedWh"] ?? 0,
                    2
                )?>
            </span>

            <span>Wh</span>

        </div>

    </div>


    <div class="live-system-card">

        <div class="system-label">
            READING TIME
        </div>

        <div class="system-value system-time">

            <span id="liveReadingTime">
                <?=htmlspecialchars(
                    $latest["time"] ?? "-"
                )?>
            </span>

        </div>

    </div>


    

</div>


<?php else: ?>

<div class="live-panel">

    <div class="no-data">
        No reading available.
    </div>

</div>

<?php endif; ?>

</div>
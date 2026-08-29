<div id="trends" class="tab-content">

    <!-- ========================================================
         TREND CONTROLS
         ======================================================== -->

    <div class="section">

        <div class="section-title">
            Trends & Analysis
        </div>

        <div class="trend-controls">

            <div class="trend-control">

                <label for="trendRange">
                    Time Range
                </label>

                <select id="trendRange">

                    <option value="1h" selected>
                        Last 1 Hour
                    </option>

                    <option value="6h">
                        Last 6 Hours
                    </option>

                    <option value="12h">
                        Last 12 Hours
                    </option>

                    <option value="24h">
                        Last 24 Hours
                    </option>

                    <option value="7d">
                        Last 7 Days
                    </option>

                    <option value="custom">
                        Custom Range
                    </option>

                </select>

            </div>


            <!-- Custom date/time -->

            <div
                id="customTrendRange"
                class="trend-custom-range"
                style="display:none;"
            >

                <div class="trend-control">

                    <label for="trendFrom">
                        From
                    </label>

                    <input
                        type="datetime-local"
                        id="trendFrom"
                    >

                </div>


                <div class="trend-control">

                    <label for="trendTo">
                        To
                    </label>

                    <input
                        type="datetime-local"
                        id="trendTo"
                    >

                </div>


                <button
                    type="button"
                    id="applyTrendRange"
                    class="trend-button"
                >
                    Apply
                </button>

            </div>

        </div>

    </div>


    <!-- ========================================================
         SUMMARY
         ======================================================== -->

    <div class="section">

        <div class="section-title">
            Summary
        </div>

        <div class="trend-summary-grid trend-summary-single-row">

            <div class="card">

                <h3>Readings</h3>

                <div class="value">
                    <span id="trendReadingCount">-</span>
                </div>

            </div>


            <div class="card">

                <h3>Average Active Power</h3>

                <div class="value">
                    <span id="trendAvgPower">-</span>
                </div>

                <div class="unit">
                    kW
                </div>

            </div>


            <div class="card">

                <h3>Maximum Active Power</h3>

                <div class="value">
                    <span id="trendMaxPower">-</span>
                </div>

                <div class="unit">
                    kW
                </div>

            </div>


            <div class="card">

                <h3>Average Current</h3>

                <div class="value">
                    <span id="trendAvgCurrent">-</span>
                </div>

                <div class="unit">
                    Amps
                </div>

            </div>


            <div class="card">

                <h3>Average Power Factor</h3>

                <div class="value">
                    <span id="trendAvgPF">-</span>
                </div>

            </div>

        </div>

    </div>


    <!-- ========================================================
         ACTIVE POWER
         ======================================================== -->

    <div class="section">

        <div
            class="section-title"
            id="powerChartTitle"
        >
            Last 1 Hour — Active Power
        </div>


        <div class="chart-card">

            <div class="chart-container">

                <canvas id="powerChart"></canvas>

            </div>

        </div>

    </div>


    <!-- ========================================================
         CURRENT
         ======================================================== -->

    <div class="section">

        <div
            class="section-title"
            id="currentChartTitle"
        >
            Last 1 Hour — Current
        </div>


        <div class="chart-card">

            <div class="chart-container">

                <canvas id="currentChart"></canvas>

            </div>

        </div>

    </div>


    <!-- ========================================================
         VOLTAGE
         ======================================================== -->

    <div class="section">

        <div
            class="section-title"
            id="voltageChartTitle"
        >
            Last 1 Hour — Voltage
        </div>


        <div class="chart-card">

            <div class="chart-container">

                <canvas id="voltageChart"></canvas>

            </div>

        </div>

    </div>


    <!-- ========================================================
         POWER FACTOR
         ======================================================== -->

    <div class="section">

        <div
            class="section-title"
            id="pfChartTitle"
        >
            Last 1 Hour — Power Factor
        </div>


        <div class="chart-card">

            <div class="chart-container">

                <canvas id="pfChart"></canvas>

            </div>

        </div>

    </div>


    <!-- ========================================================
         LINE-TO-LINE VOLTAGE
         ======================================================== -->

    <div class="section">

        <div
            class="section-title"
            id="llVoltageChartTitle"
        >
            Last 1 Hour — Line-to-Line Voltage
        </div>


        <div class="chart-card">

            <div class="chart-container">

                <canvas id="llVoltageChart"></canvas>

            </div>

        </div>

    </div>


    <!-- ========================================================
         FREQUENCY
         ======================================================== -->

    <div class="section">

        <div
            class="section-title"
            id="frequencyChartTitle"
        >
            Last 1 Hour — Frequency
        </div>


        <div class="chart-card">

            <div class="chart-container">

                <canvas id="frequencyChart"></canvas>

            </div>

        </div>

    </div>


</div>


<!-- ============================================================
     ENERGY TAB
     ============================================================ -->
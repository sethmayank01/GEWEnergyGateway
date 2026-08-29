/* ============================================================
   GEW ENERGY MONITORING - DASHBOARD.JS
   ============================================================ */


/* ============================================================
   INITIAL DATA
   ============================================================ */

const readings =
    window.GEW_DASHBOARD_READINGS || [];

const gatewayId =
    window.GEW_GATEWAY_ID || "";


/* ============================================================
   CURRENT DATA
   ============================================================ */

let currentReadings =
    readings.slice();


let trendReadings =
    [];


/* ============================================================
   CHART OBJECTS
   ============================================================ */

let chartPower = null;
let chartCurrent = null;
let chartPF = null;

let chartVoltage = null;
let chartLLVoltage = null;
let chartFrequency = null;

let chartQualityPF = null;


/* ============================================================
   TAB HANDLING
   ============================================================ */

function showTab(tabId, element)
{
    document
        .querySelectorAll(".tab-content")
        .forEach(function(tab)
        {
            tab.classList.remove("active");
        });


    document
        .querySelectorAll(".tab")
        .forEach(function(tab)
        {
            tab.classList.remove("active");
        });


    const selectedTab =
        document.getElementById(tabId);


    if(selectedTab)
    {
        selectedTab.classList.add("active");
    }


    if(element)
    {
        element.classList.add("active");
    }


    /*
     * Load Trends when the user opens the tab.
     */

    if(tabId === "trends")
    {
        loadTrends();
    }
}


/* ============================================================
   SAFE NUMBER
   ============================================================ */

function number(value)
{
    value = Number(value);

    return isNaN(value)
        ? 0
        : value;
}


/* ============================================================
   DOM HELPER
   ============================================================ */

function setText(id, value)
{
    const el =
        document.getElementById(id);

    if(el)
    {
        el.innerText = value;
    }
}


/* ============================================================
   MEASUREMENT HELPER
   ============================================================ */

function measurement(
    r,
    group,
    key
)
{
    return number(
        r &&
        r.measurements &&
        r.measurements[group] &&
        r.measurements[group][key]
    );
}


/* ============================================================
   POWER
   ============================================================ */

function power(r)
{
    return measurement(
        r,
        "power",
        "active"
    ) / 1000;
}


function reactivePower(r)
{
    return measurement(
        r,
        "power",
        "reactive"
    ) / 1000;
}


function apparentPower(r)
{
    return measurement(
        r,
        "power",
        "apparent"
    ) / 1000;
}


/* ============================================================
   POWER FACTOR
   ============================================================ */

function pf(r)
{
    return measurement(
        r,
        "powerFactor",
        "average"
    );
}


/* ============================================================
   CURRENT
   ============================================================ */

function current(
    r,
    phase
)
{
    return measurement(
        r,
        "current",
        phase
    );
}


/* ============================================================
   VOLTAGE
   ============================================================ */

function voltage(
    r,
    phase
)
{
    return measurement(
        r,
        "voltage",
        phase
    );
}


/* ============================================================
   FREQUENCY
   ============================================================ */

function frequency(r)
{
    return number(
        r &&
        r.measurements &&
        r.measurements.frequency
    );
}


/* ============================================================
   TIME LABEL
   ============================================================ */

function timeLabel(r)
{
    if(!r || !r.time)
    {
        return "";
    }


    const d =
        new Date(
            r.time.replace(" ", "T")
        );


    if(isNaN(d.getTime()))
    {
        return r.time;
    }


    return d.toLocaleTimeString(
        [],
        {
            hour:"2-digit",
            minute:"2-digit",
            second:"2-digit"
        }
    );
}


/* ============================================================
   TREND TIME LABEL
   ============================================================ */

function trendTimeLabel(r)
{
    if(!r || !r.time)
    {
        return "";
    }


    const d =
        new Date(
            r.time.replace(" ", "T")
        );


    if(isNaN(d.getTime()))
    {
        return r.time;
    }


    const range =
        document.getElementById(
            "trendRange"
        );


    const selectedRange =
        range ?
        range.value :
        "1h";


    /*
     * For longer ranges show date as well.
     */

    if(
        selectedRange === "24h" ||
        selectedRange === "7d" ||
        selectedRange === "custom"
    )
    {
        return d.toLocaleString(
            [],
            {
                day:"2-digit",
                month:"short",
                hour:"2-digit",
                minute:"2-digit"
            }
        );
    }


    return d.toLocaleTimeString(
        [],
        {
            hour:"2-digit",
            minute:"2-digit",
            second:"2-digit"
        }
    );
}


/* ============================================================
   STATISTICS
   ============================================================ */

function average(values)
{
    if(values.length === 0)
    {
        return 0;
    }


    return values.reduce(
        (a,b) => a+b,
        0
    ) / values.length;
}


function maximum(values)
{
    if(values.length === 0)
    {
        return 0;
    }


    return Math.max(...values);
}


function minimum(values)
{
    if(values.length === 0)
    {
        return 0;
    }


    return Math.min(...values);
}


/* ============================================================
   LIVE STATISTICS
   ============================================================ */

function calculateStatisticsFrom(data)
{
    if(
        !data ||
        data.length === 0
    )
    {
        return;
    }


    const powers =
        data.map(power);


    const currents =
        data.map(function(r)
        {
            return Math.max(
                current(r,"l1"),
                current(r,"l2"),
                current(r,"l3")
            );
        });


    const pfs =
        data.map(pf);


    const l1 =
        data.map(
            r => current(r,"l1")
        );


    const l2 =
        data.map(
            r => current(r,"l2")
        );


    const l3 =
        data.map(
            r => current(r,"l3")
        );


    const kva =
        data.map(
            apparentPower
        );


    setText(
        "avgPower",
        average(powers).toFixed(2)
    );


    setText(
        "maxPower",
        maximum(powers).toFixed(2)
    );


    setText(
        "minPower",
        minimum(powers).toFixed(2)
    );


    const maxCurrent =
        maximum(currents);


    setText(
        "maxCurrent",
        maxCurrent.toFixed(2) + " A"
    );


    setText(
        "avgCurrent",
        average(currents).toFixed(2) + " A"
    );


    setText(
        "maxL1",
        maximum(l1).toFixed(2)
    );


    setText(
        "maxL2",
        maximum(l2).toFixed(2)
    );


    setText(
        "maxL3",
        maximum(l3).toFixed(2)
    );


    setText(
        "maxKVA",
        maximum(kva).toFixed(2)
    );


    setText(
        "avgPF",
        average(pfs).toFixed(3)
    );


    setText(
        "qualityAvgPF",
        average(pfs).toFixed(3)
    );


    setText(
        "minPF",
        minimum(pfs).toFixed(3)
    );


    setText(
        "maxPF",
        maximum(pfs).toFixed(3)
    );


    setText(
        "readingCount",
        data.length
    );


    setText(
        "avgV1",
        average(
            data.map(
                r => voltage(r,"l1")
            )
        ).toFixed(2)
    );


    setText(
        "avgV2",
        average(
            data.map(
                r => voltage(r,"l2")
            )
        ).toFixed(2)
    );


    setText(
        "avgV3",
        average(
            data.map(
                r => voltage(r,"l3")
            )
        ).toFixed(2)
    );


    setText(
        "avgFreq",
        average(
            data.map(frequency)
        ).toFixed(2)
    );
}


/* ============================================================
   UPDATE LIVE DISPLAY
   ============================================================ */

function updateLiveDisplay(
    latest,
    gatewayData
)
{
    if(!latest)
    {
        return;
    }


    setText(
        "liveV1",
        voltage(latest,"l1").toFixed(2)
    );


    setText(
        "liveV2",
        voltage(latest,"l2").toFixed(2)
    );


    setText(
        "liveV3",
        voltage(latest,"l3").toFixed(2)
    );


    setText(
        "liveVNavg",
        measurement(
            latest,
            "voltage",
            "lnAverage"
        ).toFixed(2)
    );


    setText(
        "liveVNavg2",
        measurement(
            latest,
            "voltage",
            "lnAverage"
        ).toFixed(2)
    );


    setText(
        "liveVLLavg",
        measurement(
            latest,
            "voltage",
            "llAverage"
        ).toFixed(2)
    );


    setText(
        "liveVLLavg2",
        measurement(
            latest,
            "voltage",
            "llAverage"
        ).toFixed(2)
    );


    setText(
        "liveV12",
        measurement(
            latest,
            "voltage",
            "l12"
        ).toFixed(2)
    );


    setText(
        "liveV23",
        measurement(
            latest,
            "voltage",
            "l23"
        ).toFixed(2)
    );


    setText(
        "liveV31",
        measurement(
            latest,
            "voltage",
            "l31"
        ).toFixed(2)
    );


    setText(
        "liveFreq",
        frequency(latest).toFixed(2)
    );


    setText(
        "liveI1",
        current(latest,"l1").toFixed(2)
    );


    setText(
        "liveI2",
        current(latest,"l2").toFixed(2)
    );


    setText(
        "liveI3",
        current(latest,"l3").toFixed(2)
    );


    setText(
        "liveIAvg",
        measurement(
            latest,
            "current",
            "average"
        ).toFixed(2)
    );


    setText(
        "liveIAvg2",
        measurement(
            latest,
            "current",
            "average"
        ).toFixed(2)
    );


    setText(
        "livePF",
        pf(latest).toFixed(3)
    );


    setText(
        "livePF2",
        pf(latest).toFixed(3)
    );


    setText(
        "liveKW",
        power(latest).toFixed(2)
    );


    setText(
        "liveKVAR",
        reactivePower(latest).toFixed(2)
    );


    setText(
        "liveKVA",
        apparentPower(latest).toFixed(2)
    );


    setText(
        "livePFL1",
        measurement(
            latest,
            "powerFactor",
            "l1"
        ).toFixed(3)
    );


    setText(
        "livePFL2",
        measurement(
            latest,
            "powerFactor",
            "l2"
        ).toFixed(3)
    );


    setText(
        "livePFL3",
        measurement(
            latest,
            "powerFactor",
            "l3"
        ).toFixed(3)
    );


    setText(
        "liveEnergy",
        number(
            latest.measurements &&
            latest.measurements.energyReceivedWh
        ).toFixed(2)
    );


    setText(
        "liveReadingTime",
        latest.time || "-"
    );


    setText(
        "liveSequence",
        latest.sequence ?? "-"
    );


    /*
     * IMPORTANT:
     *
     * Last Seen is ONLY the time of the latest
     * meter reading.
     *
     * It is NOT the gateway ONLINE status.
     */

    setText(
        "pageLastSeen",
        latest.time || "-"
    );


    setText(
        "liveLastSeen",
        latest.time || "-"
    );


    if(gatewayData)
    {
        setText(
            "liveFirmware",
            gatewayData.firmware || "-"
        );
    }
}


/* ============================================================
   UPDATE GATEWAY STATUS
   ============================================================ */

function updateGatewayStatus(
    gatewayData
)
{
    if(!gatewayData)
    {
        return;
    }


    const statusText =
        String(
            gatewayData.status ||
            "UNKNOWN"
        );


    const online =
        statusText.toLowerCase() ===
        "online";


    /*
     * IMPORTANT:
     *
     * Only update the element with ID
     * pageGatewayStatus.
     *
     * Do NOT use:
     * .gateway-status span:first-child
     *
     * because that was causing Last Seen
     * to be overwritten.
     */

    const el =
        document.getElementById(
            "pageGatewayStatus"
        );


    if(!el)
    {
        return;
    }


    el.classList.remove(
        "status-online",
        "status-offline"
    );


    el.classList.add(
        online
            ? "status-online"
            : "status-offline"
    );


    el.innerText =
        "● " + statusText;
}


/* ============================================================
   UPDATE BROWSER REFRESH TIME
   ============================================================ */

function updateDataRefreshTime()
{
    const now =
        new Date();


    setText(
        "pageRefreshTime",
        now.toLocaleString(
            "en-IN",
            {
                day:"2-digit",
                month:"short",
                year:"numeric",
                hour:"2-digit",
                minute:"2-digit",
                second:"2-digit",
                hour12:false
            }
        )
    );
}


/* ============================================================
   TREND RANGE LABEL
   ============================================================ */

function trendRangeLabel()
{
    const select =
        document.getElementById(
            "trendRange"
        );


    if(!select)
    {
        return "Last 1 Hour";
    }


    if(
        select.value === "custom"
    )
    {
        const from =
            document.getElementById(
                "trendFrom"
            );

        const to =
            document.getElementById(
                "trendTo"
            );


        if(
            from &&
            to &&
            from.value &&
            to.value
        )
        {
            return (
                formatDateTime(from.value) +
                " – " +
                formatDateTime(to.value)
            );
        }


        return "Custom Range";
    }


    return select.options[
        select.selectedIndex
    ].text;
}


/* ============================================================
   FORMAT CUSTOM DATE
   ============================================================ */

function formatDateTime(value)
{
    if(!value)
    {
        return "-";
    }


    const d =
        new Date(value);


    if(isNaN(d.getTime()))
    {
        return value;
    }


    return d.toLocaleString(
        "en-IN",
        {
            day:"2-digit",
            month:"short",
            year:"numeric",
            hour:"2-digit",
            minute:"2-digit",
            hour12:false
        }
    );
}


/* ============================================================
   UPDATE TREND TITLES
   ============================================================ */

function updateTrendTitles()
{
    const label =
        trendRangeLabel();


    setText(
        "powerChartTitle",
        label + " — Active Power"
    );


    setText(
        "currentChartTitle",
        label + " — Current"
    );


    setText(
        "voltageChartTitle",
        label + " — Voltage"
    );


    setText(
        "llVoltageChartTitle",
        label + " — Line-to-Line Voltage"
    );


    setText(
        "pfChartTitle",
        label + " — Power Factor"
    );


    setText(
        "frequencyChartTitle",
        label + " — Frequency"
    );
}


/* ============================================================
   DESTROY CHART
   ============================================================ */

function destroyChart(chart)
{
    if(chart)
    {
        chart.destroy();
    }

    return null;
}


/* ============================================================
   CREATE MULTI-DATASET CHART
   ============================================================ */

function createTrendChart(
    canvasId,
    datasets,
    unit,
    readingsData
)
{
    const canvas =
        document.getElementById(
            canvasId
        );


    if(!canvas)
    {
        return null;
    }


    const labels =
        readingsData.map(
            trendTimeLabel
        );


    /*
     * We retain EVERY reading.
     *
     * For large datasets we only hide the
     * individual point markers.
     *
     * No data is removed.
     */

    const pointRadius =
        readingsData.length > 1000
            ? 0
            : 1;


    return new Chart(
        canvas,
        {
            type:"line",

            data:
            {
                labels:labels,

                datasets:
                    datasets.map(
                        function(ds)
                        {
                            return {

                                label:ds.label,

                                data:
                                    ds.data,

                                borderWidth:2,

                                pointRadius:
                                    pointRadius,

                                pointHoverRadius:4,

                                tension:0.15,

                                fill:false
                            };
                        }
                    )
            },


            options:
            {
                responsive:true,

                maintainAspectRatio:false,

                animation:false,

                interaction:
                {
                    mode:"index",

                    intersect:false
                },


                plugins:
                {
                    legend:
                    {
                        display:true
                    },


                    tooltip:
                    {
                        mode:"index",

                        intersect:false
                    }
                },


                scales:
                {
                    x:
                    {
                        ticks:
                        {
                            color:"#94a3b8",

                            maxTicksLimit:10,

                            autoSkip:true
                        },


                        grid:
                        {
                            color:"#334155"
                        }
                    },


                    y:
                    {
                        title:
                        {
                            display:true,

                            text:unit,

                            color:"#94a3b8"
                        },


                        ticks:
                        {
                            color:"#94a3b8"
                        },


                        grid:
                        {
                            color:"#334155"
                        }
                    }
                }
            }
        }
    );
}


/* ============================================================
   LOAD TRENDS API
   ============================================================ */

async function loadTrends()
{
    const rangeSelect =
        document.getElementById(
            "trendRange"
        );


    if(!rangeSelect)
    {
        return;
    }


    const range =
        rangeSelect.value;

    let url =
           "/energy/api/trends.php" +
        "?gatewayId=" +
        encodeURIComponent(
            gatewayId
        ) +
        "&range=" +
        encodeURIComponent(
            range
        );

    /*
     * Custom range
     */

    if(range === "custom")
    {
        const from =
            document.getElementById(
                "trendFrom"
            );


        const to =
            document.getElementById(
                "trendTo"
            );


        if(
            !from ||
            !to ||
            !from.value ||
            !to.value
        )
        {
            return;
        }


        url +=
            "&from=" +
            encodeURIComponent(
                from.value.replace("T"," ")
            );


        url +=
            "&to=" +
            encodeURIComponent(
                to.value.replace("T"," ")
            );
    }


    url +=
        "&_=" +
        Date.now();


    try
    {
        const response =
            await fetch(
                url,
                {
                    cache:"no-store"
                }
            );


        if(!response.ok)
        {
            throw new Error(
                "Trends API HTTP " +
                response.status
            );
        }


        const data =
            await response.json();


        if(
            data.status !== "OK"
        )
        {
            throw new Error(
                data.message ||
                "Trends API error"
            );
        }


        trendReadings =
            Array.isArray(
                data.readings
            )
            ? data.readings
            : [];

         //Mayank
        console.log(
    "Trend readings:",
    trendReadings
);

console.log(
    "Trend count:",
    trendReadings.length
);

console.log(
    "First reading:",
    trendReadings[0]
);

        /*
         * API returns chronological data.
         * Keep it exactly as returned.
         */

        renderTrendCharts();


        updateTrendSummary();


        updateTrendTitles();


        /*
         * Display the actual count returned
         * by the API.
         */

        setText(
            "trendReadingCount",
            trendReadings.length
        );
    }
    catch(error)
    {
        console.error(
            "Trend loading failed:",
            error
        );

      alert(
        "Unable to load trend data.\n\n" +
        error.message
    );
    }
}


/* ============================================================
   RENDER TREND CHARTS
   ============================================================ */

function renderTrendCharts()
{
    chartPower =
        destroyChart(
            chartPower
        );


    chartCurrent =
        destroyChart(
            chartCurrent
        );


    chartVoltage =
        destroyChart(
            chartVoltage
        );


    chartLLVoltage =
        destroyChart(
            chartLLVoltage
        );


    chartPF =
        destroyChart(
            chartPF
        );


    chartFrequency =
        destroyChart(
            chartFrequency
        );


    /*
     * ACTIVE POWER
     */

    chartPower =
        createTrendChart(
            "powerChart",

            [
                {
                    label:"Active Power",

                    data:
                        trendReadings.map(
                            power
                        )
                },

                {
                    label:"Reactive Power",

                    data:
                        trendReadings.map(
                            reactivePower
                        )
                },

                {
                    label:"Apparent Power",

                    data:
                        trendReadings.map(
                            apparentPower
                        )
                }
            ],

            "kW / kVAr / kVA",

            trendReadings
        );


    /*
     * CURRENT
     */

    chartCurrent =
        createTrendChart(
            "currentChart",

            [
                {
                    label:"L1",

                    data:
                        trendReadings.map(
                            r =>
                                current(r,"l1")
                        )
                },

                {
                    label:"L2",

                    data:
                        trendReadings.map(
                            r =>
                                current(r,"l2")
                        )
                },

                {
                    label:"L3",

                    data:
                        trendReadings.map(
                            r =>
                                current(r,"l3")
                        )
                },

                {
                    label:"Average",

                    data:
                        trendReadings.map(
                            r =>
                                current(
                                    r,
                                    "average"
                                )
                        )
                }
            ],

            "A",

            trendReadings
        );


    /*
     * LINE-TO-NEUTRAL VOLTAGE
     */

    chartVoltage =
        createTrendChart(
            "voltageChart",

            [
                {
                    label:"L1",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "l1"
                                )
                        )
                },

                {
                    label:"L2",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "l2"
                                )
                        )
                },

                {
                    label:"L3",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "l3"
                                )
                        )
                },

                {
                    label:"L-N Average",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "lnAverage"
                                )
                        )
                }
            ],

            "V",

            trendReadings
        );


    /*
     * LINE-TO-LINE VOLTAGE
     */

    chartLLVoltage =
        createTrendChart(
            "llVoltageChart",

            [
                {
                    label:"L12",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "l12"
                                )
                        )
                },

                {
                    label:"L23",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "l23"
                                )
                        )
                },

                {
                    label:"L31",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "l31"
                                )
                        )
                },

                {
                    label:"L-L Average",

                    data:
                        trendReadings.map(
                            r =>
                                voltage(
                                    r,
                                    "llAverage"
                                )
                        )
                }
            ],

            "V",

            trendReadings
        );


    /*
     * POWER FACTOR
     */

    chartPF =
        createTrendChart(
            "pfChart",

            [
                {
                    label:"Average",

                    data:
                        trendReadings.map(
                            r =>
                                measurement(
                                    r,
                                    "powerFactor",
                                    "average"
                                )
                        )
                },

                {
                    label:"L1",

                    data:
                       trendReadings.map(
                        function(r)
                        {
                            return Math.abs(
                                measurement(
                                    r,
                                    "powerFactor",
                                    "l1"
                                )
                            );
                        }
                    )
                },

                {
                    label:"L2",

                    data:
                       trendReadings.map(
                        function(r)
                        {
                            return Math.abs(
                                measurement(
                                    r,
                                    "powerFactor",
                                    "l2"
                                )
                            );
                        }
                    )
                },

                {
                    label:"L3",

                    data:
                        trendReadings.map(
                        function(r)
                        {
                            return Math.abs(
                                measurement(
                                    r,
                                    "powerFactor",
                                    "l3"
                                )
                            );
                        }
                    )
                }
            ],

            "Power Factor",

            trendReadings
        );


    /*
     * FREQUENCY
     */

    chartFrequency =
        createTrendChart(
            "frequencyChart",

            [
                {
                    label:"Frequency",

                    data:
                        trendReadings.map(
                            frequency
                        )
                }
            ],

            "Hz",

            trendReadings
        );
}


/* ============================================================
   TREND SUMMARY
   ============================================================ */

function updateTrendSummary()
{
    if(
        trendReadings.length === 0
    )
    {
        setText(
            "trendReadingCount",
            "0"
        );

        setText(
            "trendAvgPower",
            "-"
        );

        setText(
            "trendMaxPower",
            "-"
        );

        setText(
            "trendAvgCurrent",
            "-"
        );

        setText(
            "trendAvgPF",
            "-"
        );

        return;
    }


    const powers =
        trendReadings.map(
            power
        );


    const currents =
        trendReadings.map(
            r =>
                current(
                    r,
                    "average"
                )
        );


    const pfs = trendReadings.map(function(r)
{
    const active =
        Math.abs(
            measurement(
                r,
                "power",
                "active"
            )
        );

    const apparent =
        Math.abs(
            measurement(
                r,
                "power",
                "apparent"
            )
        );

    if(apparent === 0)
    {
        return 0;
    }

    return active / apparent;
});


    setText(
        "trendReadingCount",
        trendReadings.length
    );


    setText(
        "trendAvgPower",
        average(powers).toFixed(2)
    );


    setText(
        "trendMaxPower",
        maximum(powers).toFixed(2)
    );


    setText(
        "trendAvgCurrent",
        average(currents).toFixed(2)
    );


    setText(
        "trendAvgPF",
        average(pfs).toFixed(3)
    );
}


/* ============================================================
   TREND RANGE EVENTS
   ============================================================ */

function setupTrendControls()
{
    const rangeSelect =
        document.getElementById(
            "trendRange"
        );


    const customRange =
        document.getElementById(
            "customTrendRange"
        );


    if(!rangeSelect)
    {
        return;
    }


    rangeSelect.addEventListener(
        "change",
        function()
        {
            if(customRange)
            {
                customRange.style.display =
                    this.value === "custom"
                        ? "flex"
                        : "none";
            }


            if(this.value !== "custom")
            {
                updateTrendTitles();

                loadTrends();
            }
            else
            {
                updateTrendTitles();
            }
        }
    );


    const applyButton =
        document.getElementById(
            "applyTrendRange"
        );


    if(applyButton)
    {
        applyButton.addEventListener(
            "click",
            function()
            {
                loadTrends();
            }
        );
    }
}


/* ============================================================
   INITIALISE TREND CONTROLS
   ============================================================ */

setupTrendControls();


/* ============================================================
   INITIALISE LIVE DATA
   ============================================================ */

if(
    currentReadings.length > 0
)
{
    calculateStatisticsFrom(
        currentReadings
    );
}


/* ============================================================
   INITIALISE EXISTING QUALITY PF CHART
   ============================================================ */

function initialiseQualityPFChart()
{
    const canvas =
        document.getElementById(
            "qualityPFChart"
        );


    if(!canvas)
    {
        return;
    }


    chartQualityPF =
        createTrendChart(
            "qualityPFChart",

            [
                {
                    label:"Power Factor",

                    data:
                        currentReadings.map(
                            pf
                        )
                }
            ],

            "PF",

            currentReadings
        );
}


/* ============================================================
   UPDATE EXISTING QUALITY CHART
   ============================================================ */

function refreshQualityPFChart()
{
    if(!chartQualityPF)
    {
        return;
    }


    const chronological =
        currentReadings
            .slice()
            .reverse();


    chartQualityPF.data.labels =
        chronological.map(
            timeLabel
        );


    chartQualityPF.data.datasets[0].data =
        chronological.map(
            pf
        );


    chartQualityPF.update(
        "none"
    );
}


/* ============================================================
   FETCH LIVE DATA
   ============================================================ */

async function refreshDashboard()
{
    try
    {
        const cacheBuster =
            "&_=" +
            Date.now();


        const readingUrl =
            "/energy/api/readings.php" +
            "?gatewayId=" +
            encodeURIComponent(
                gatewayId
            ) +
            "&limit=120" +
            cacheBuster;


        const statusUrl =
            "/energy/api/status.php" +
            "?gatewayId=" +
            encodeURIComponent(
                gatewayId
            ) +
            "&_=" +
            Date.now();


        const results =
            await Promise.allSettled(
                [
                    fetch(
                        readingUrl,
                        {
                            cache:"no-store"
                        }
                    ),

                    fetch(
                        statusUrl,
                        {
                            cache:"no-store"
                        }
                    )
                ]
            );


        /* ====================================================
           READING API
           ==================================================== */

        let readingData = null;


        if(
            results[0].status ===
            "fulfilled" &&
            results[0].value.ok
        )
        {
            readingData =
                await results[0]
                    .value
                    .json();
        }
        else
        {
            console.warn(
                "Reading API unavailable"
            );
        }


        /* ====================================================
           STATUS API
           ==================================================== */

        let statusData = null;


        if(
            results[1].status ===
            "fulfilled" &&
            results[1].value.ok
        )
        {
            statusData =
                await results[1]
                    .value
                    .json();
        }
        else
        {
            console.warn(
                "Status API unavailable"
            );
        }


        const newReadings =
            readingData &&
            readingData.readings
                ? readingData.readings
                : [];


        const gatewayData =
            statusData &&
            statusData.gateways
                ? statusData.gateways[0]
                : null;


        /*
         * Always update gateway status if
         * status API succeeded.
         */

        updateGatewayStatus(
            gatewayData
        );


        /*
         * If readings API failed, don't destroy
         * the existing Live display.
         */

        if(
            newReadings.length === 0
        )
        {
            console.warn(
                "No new readings received"
            );

            updateDataRefreshTime();

            return;
        }


        currentReadings =
            newReadings;


        const latest =
            currentReadings[0];


        /*
         * Update Live tab.
         */

        updateLiveDisplay(
            latest,
            gatewayData
        );


        /*
         * Update statistics.
         */

        calculateStatisticsFrom(
            currentReadings
        );


        /*
         * Update existing Power Quality
         * chart.
         */

        refreshQualityPFChart();


        /*
         * Browser refresh time.
         */

        updateDataRefreshTime();
    }
    catch(error)
    {
        console.error(
            "Dashboard update failed:",
            error
        );
    }
}


/* ============================================================
   FIRST PAGE LOAD
   ============================================================ */

updateDataRefreshTime();


/*
 * Create Power Quality chart after DOM is ready.
 */

initialiseQualityPFChart();


/*
 * Load the default Trends range.
 */

if(
    document.getElementById(
        "trendRange"
    )
)
{
    loadTrends();
}


/* ============================================================
   AUTOMATIC LIVE UPDATE
   ============================================================ */

/*
 * Every 30 seconds:
 *
 * - Live readings updated
 * - Gateway status updated
 * - Last Seen updated from latest reading
 * - Browser refresh time updated
 *
 * Page itself is NEVER reloaded.
 *
 * Trends are not automatically reloaded every 30 seconds,
 * because a 7-day request could contain tens of thousands
 * of readings. Trends reload when:
 *
 * - Trends tab is opened
 * - Range is changed
 * - Custom Apply is pressed
 */

setInterval(
    refreshDashboard,
    30000
);

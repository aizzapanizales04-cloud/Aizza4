// =====================================================
// AIZZA ACT4
// DHT11 FIREBASE ENVIRONMENT MONITOR
// =====================================================

import {
    initializeApp
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";

import {
    getDatabase,
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";


// =====================================================
// FIREBASE CONFIG
// =====================================================

const firebaseConfig = {

    apiKey:
        "AIzaSyCmdlAD8VWAhfQH_sS7ACSviLWF3M8IGak",

    authDomain:
        "aizza-3d822.firebaseapp.com",

    databaseURL:
        "https://aizza-3d822-default-rtdb.firebaseio.com",

    projectId:
        "aizza-3d822",

    storageBucket:
        "aizza-3d822.firebasestorage.app",

    messagingSenderId:
        "715346081633",

    appId:
        "1:715346081633:web:fe86bcb069f53868ffb720",

    measurementId:
        "G-YPZRFYY9YY"

};


// =====================================================
// INITIALIZE FIREBASE
// =====================================================

const app =
    initializeApp(firebaseConfig);

const db =
    getDatabase(app);


// =====================================================
// DATABASE REFERENCE
// =====================================================

const dataRef =
    ref(db, "ESP32_Data");


// =====================================================
// HTML ELEMENTS
// =====================================================

const firebaseStatus =
    document.getElementById(
        "firebaseStatus"
    );

const firebaseStatusDot =
    document.getElementById(
        "firebaseStatusDot"
    );

const currentTemperature =
    document.getElementById(
        "currentTemperature"
    );

const currentHumidity =
    document.getElementById(
        "currentHumidity"
    );

const graphDate =
    document.getElementById(
        "graphDate"
    );

const historyDate =
    document.getElementById(
        "historyDate"
    );

const historyTableBody =
    document.getElementById(
        "historyTableBody"
    );

const recordCount =
    document.getElementById(
        "recordCount"
    );

const toggleHistory =
    document.getElementById(
        "toggleHistory"
    );

const historyContent =
    document.getElementById(
        "historyContent"
    );

const message =
    document.getElementById(
        "message"
    );


// =====================================================
// VARIABLES
// =====================================================

let allSensorData = {};

let selectedDate = "";

let sensorChart = null;


// =====================================================
// NUMBER HELPER
// =====================================================

function toNumber(value) {

    const number =
        Number(value);


    if (
        Number.isFinite(number)
    ) {

        return number;
    }


    return null;
}


// =====================================================
// FORMAT NUMBER
// =====================================================

function formatNumber(value) {

    if (
        value === null ||
        value === undefined
    ) {

        return "--";
    }


    return Number(value).toFixed(1);
}


// =====================================================
// STATUS
// =====================================================

function setFirebaseStatus(
    text,
    connected
) {

    if (firebaseStatus) {

        firebaseStatus.textContent =
            text;
    }


    if (firebaseStatusDot) {

        if (connected) {

            firebaseStatusDot.classList.add(
                "online"
            );

        } else {

            firebaseStatusDot.classList.remove(
                "online"
            );
        }
    }
}


// =====================================================
// GET DATE LIST
// =====================================================

function getDateList(data) {

    return Object.keys(
        data || {}
    )
    .filter(
        key => {

            return (
                data[key] &&
                typeof data[key] === "object"
            );

        }
    )
    .sort()
    .reverse();
}


// =====================================================
// GET LATEST READING
// =====================================================

function getLatestReading(data) {

    let latest = null;


    const dates =
        Object.keys(
            data || {}
        ).sort();


    for (
        const date of dates
    ) {

        const times =
            Object.keys(
                data[date] || {}
            ).sort();


        for (
            const time of times
        ) {

            const reading =
                data[date][time];


            if (
                !reading ||
                typeof reading !== "object"
            ) {

                continue;
            }


            const temperature =
                toNumber(
                    reading.temperature
                );


            const humidity =
                toNumber(
                    reading.humidity
                );


            if (
                temperature === null &&
                humidity === null
            ) {

                continue;
            }


            latest = {

                date:
                    date,

                time:
                    time,

                temperature:
                    temperature,

                humidity:
                    humidity

            };
        }
    }


    return latest;
}


// =====================================================
// GET READINGS FOR DATE
// =====================================================

function getReadingsForDate(
    date
) {

    const result = [];


    if (!date) {

        return result;
    }


    const dayData =
        allSensorData[date];


    if (
        !dayData ||
        typeof dayData !== "object"
    ) {

        return result;
    }


    const times =
        Object.keys(dayData)
            .filter(
                time => {

                    return (
                        dayData[time] &&
                        typeof dayData[time] === "object"
                    );

                }
            )
            .sort();


    times.forEach(
        time => {

            const reading =
                dayData[time];


            const temperature =
                toNumber(
                    reading.temperature
                );


            const humidity =
                toNumber(
                    reading.humidity
                );


            if (
                temperature === null &&
                humidity === null
            ) {

                return;
            }


            result.push({

                time:
                    time,

                temperature:
                    temperature,

                humidity:
                    humidity

            });

        }
    );


    return result;
}


// =====================================================
// POPULATE DATE SELECTS
// =====================================================

function populateDateSelects() {

    const dates =
        getDateList(
            allSensorData
        );


    // -------------------------------------------------
    // GRAPH DATE
    // -------------------------------------------------

    if (graphDate) {

        graphDate.innerHTML = "";


        if (
            dates.length === 0
        ) {

            const option =
                document.createElement(
                    "option"
                );

            option.value = "";

            option.textContent =
                "No dates available";

            graphDate.appendChild(
                option
            );

        } else {

            dates.forEach(
                date => {

                    const option =
                        document.createElement(
                            "option"
                        );

                    option.value =
                        date;

                    option.textContent =
                        date;

                    graphDate.appendChild(
                        option
                    );

                }
            );


            graphDate.value =
                selectedDate || dates[0];
        }
    }


    // -------------------------------------------------
    // HISTORY DATE
    // -------------------------------------------------

    if (historyDate) {

        historyDate.innerHTML = "";


        if (
            dates.length === 0
        ) {

            const option =
                document.createElement(
                    "option"
                );

            option.value = "";

            option.textContent =
                "No dates available";

            historyDate.appendChild(
                option
            );

        } else {

            dates.forEach(
                date => {

                    const option =
                        document.createElement(
                            "option"
                        );

                    option.value =
                        date;

                    option.textContent =
                        date;

                    historyDate.appendChild(
                        option
                    );

                }
            );


            historyDate.value =
                selectedDate || dates[0];
        }
    }
}


// =====================================================
// UPDATE CURRENT READING
// =====================================================

function updateCurrentReading() {

    const latest =
        getLatestReading(
            allSensorData
        );


    if (!latest) {

        currentTemperature.textContent =
            "-- °C";

        currentHumidity.textContent =
            "-- %";

        return;
    }


    currentTemperature.textContent =
        formatNumber(
            latest.temperature
        ) +
        " °C";


    currentHumidity.textContent =
        formatNumber(
            latest.humidity
        ) +
        " %";


    console.log(
        "LATEST:",
        latest
    );
}


// =====================================================
// UPDATE CHART
// =====================================================

function updateChart() {

    if (
        typeof Chart === "undefined"
    ) {

        console.error(
            "Chart.js is not loaded."
        );

        return;
    }


    const date =
        graphDate
            ? graphDate.value
            : selectedDate;


    const readings =
        getReadingsForDate(
            date
        );


    const labels =
        readings.map(
            item => item.time
        );


    const temperatures =
        readings.map(
            item => item.temperature
        );


    const humidities =
        readings.map(
            item => item.humidity
        );


    const canvas =
        document.getElementById(
            "sensorChart"
        );


    if (!canvas) {

        return;
    }


    if (sensorChart) {

        sensorChart.destroy();

        sensorChart = null;
    }


    sensorChart =
        new Chart(
            canvas,
            {

                type: "line",


                data: {

                    labels:
                        labels,

                    datasets: [

                        {

                            label:
                                "Temperature (°C)",

                            data:
                                temperatures,

                            borderColor:
                                "#111",

                            backgroundColor:
                                "rgba(17,17,17,0.06)",

                            yAxisID:
                                "temperature",

                            tension:
                                0.3,

                            borderWidth:
                                2,

                            pointRadius:
                                3,

                            pointBackgroundColor:
                                "#111"

                        },


                        {

                            label:
                                "Humidity (%)",

                            data:
                                humidities,

                            borderColor:
                                "#777",

                            backgroundColor:
                                "rgba(119,119,119,0.06)",

                            yAxisID:
                                "humidity",

                            tension:
                                0.3,

                            borderWidth:
                                2,

                            pointRadius:
                                3,

                            pointBackgroundColor:
                                "#777"

                        }

                    ]

                },


                options: {

                    responsive:
                        true,

                    maintainAspectRatio:
                        false,

                    interaction: {

                        mode:
                            "index",

                        intersect:
                            false

                    },


                    plugins: {

                        legend: {

                            labels: {

                                usePointStyle:
                                    true,

                                color:
                                    "#555"

                            }

                        }

                    },


                    scales: {

                        x: {

                            ticks: {

                                color:
                                    "#888"

                            },

                            grid: {

                                color:
                                    "#f0f0f0"

                            }

                        },


                        temperature: {

                            type:
                                "linear",

                            position:
                                "left",

                            title: {

                                display:
                                    true,

                                text:
                                    "Temperature (°C)"

                            },

                            ticks: {

                                color:
                                    "#555"

                            },

                            grid: {

                                color:
                                    "#eeeeee"

                            }

                        },


                        humidity: {

                            type:
                                "linear",

                            position:
                                "right",

                            title: {

                                display:
                                    true,

                                text:
                                    "Humidity (%)"

                            },

                            ticks: {

                                color:
                                    "#777"

                            },

                            grid: {

                                drawOnChartArea:
                                    false

                            }

                        }

                    }

                }

            }
        );
}


// =====================================================
// UPDATE HISTORY
// =====================================================

function updateHistory() {

    if (!historyTableBody) {

        return;
    }


    const date =
        historyDate
            ? historyDate.value
            : selectedDate;


    const readings =
        getReadingsForDate(
            date
        );


    historyTableBody.innerHTML =
        "";


    if (
        readings.length === 0
    ) {

        historyTableBody.innerHTML = `

            <tr>

                <td
                    colspan="3"
                    class="empty"
                >
                    No sensor data available.
                </td>

            </tr>

        `;


        recordCount.textContent =
            "0 records";


        return;
    }


    // Newest first

    readings
        .slice()
        .reverse()
        .forEach(
            reading => {

                const row =
                    document.createElement(
                        "tr"
                    );


                const timeCell =
                    document.createElement(
                        "td"
                    );


                const temperatureCell =
                    document.createElement(
                        "td"
                    );


                const humidityCell =
                    document.createElement(
                        "td"
                    );


                timeCell.textContent =
                    reading.time;


                temperatureCell.textContent =
                    formatNumber(
                        reading.temperature
                    ) +
                    " °C";


                humidityCell.textContent =
                    formatNumber(
                        reading.humidity
                    ) +
                    " %";


                row.appendChild(
                    timeCell
                );

                row.appendChild(
                    temperatureCell
                );

                row.appendChild(
                    humidityCell
                );


                historyTableBody.appendChild(
                    row
                );

            }
        );


    recordCount.textContent =
        readings.length +
        (
            readings.length === 1
                ? " record"
                : " records"
        );
}


// =====================================================
// UPDATE DASHBOARD
// =====================================================

function updateDashboard() {

    const dates =
        getDateList(
            allSensorData
        );


    if (
        dates.length === 0
    ) {

        currentTemperature.textContent =
            "-- °C";

        currentHumidity.textContent =
            "-- %";


        historyTableBody.innerHTML = `

            <tr>

                <td
                    colspan="3"
                    class="empty"
                >
                    Waiting for sensor records...
                </td>

            </tr>

        `;


        recordCount.textContent =
            "0 records";


        updateChart();

        return;
    }


    if (
        !selectedDate ||
        !dates.includes(
            selectedDate
        )
    ) {

        selectedDate =
            dates[0];
    }


    populateDateSelects();

    updateCurrentReading();

    updateChart();

    updateHistory();
}


// =====================================================
// FIREBASE LISTENER
// =====================================================

setFirebaseStatus(
    "CONNECTING",
    false
);


onValue(

    dataRef,

    snapshot => {

        try {

            const value =
                snapshot.val();


            allSensorData =
                value || {};


            setFirebaseStatus(
                "FIREBASE ONLINE",
                true
            );


            if (message) {

                message.textContent =
                    "Firebase data synchronized.";
            }


            updateDashboard();


            console.log(
                "Firebase data received:",
                allSensorData
            );

        } catch (error) {

            console.error(
                "Dashboard update error:",
                error
            );

            setFirebaseStatus(
                "ERROR",
                false
            );
        }

    },


    error => {

        console.error(
            "Firebase read error:",
            error
        );


        setFirebaseStatus(
            "FIREBASE ERROR",
            false
        );


        if (message) {

            message.textContent =
                "Unable to read Firebase data.";
        }
    }

);


// =====================================================
// GRAPH DATE CHANGE
// =====================================================

if (graphDate) {

    graphDate.addEventListener(
        "change",
        function() {

            selectedDate =
                this.value;


            if (historyDate) {

                historyDate.value =
                    selectedDate;
            }


            updateChart();

            updateHistory();

        }
    );
}


// =====================================================
// HISTORY DATE CHANGE
// =====================================================

if (historyDate) {

    historyDate.addEventListener(
        "change",
        function() {

            selectedDate =
                this.value;


            if (graphDate) {

                graphDate.value =
                    selectedDate;
            }


            updateHistory();

            updateChart();

        }
    );
}


// =====================================================
// SHOW / HIDE HISTORY
// =====================================================

if (toggleHistory) {

    toggleHistory.addEventListener(
        "click",
        function() {

            if (
                historyContent.classList.contains(
                    "hidden"
                )
            ) {

                historyContent.classList.remove(
                    "hidden"
                );


                toggleHistory.textContent =
                    "HIDE HISTORY";


                updateHistory();

            } else {

                historyContent.classList.add(
                    "hidden"
                );


                toggleHistory.textContent =
                    "SHOW HISTORY";
            }

        }
    );
}


// =====================================================
// INITIAL
// =====================================================

console.log(
    "================================="
);

console.log(
    "AIZZA ACT4 DHT11 MONITOR"
);

console.log(
    "Firebase:",
    firebaseConfig.projectId
);

console.log(
    "Database path: /ESP32_Data"
);

console.log(
    "================================="
);

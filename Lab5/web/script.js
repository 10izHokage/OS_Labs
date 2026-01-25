const API_URL = "http://localhost:8080";

let chart = null;
let currentPeriod = "hourly";
let historyData = [];

async function fetchJSON(url) {
    const response = await fetch(url);
    return response.json();
}

async function updateCurrent() {
    try {
        const data = await fetchJSON(`${API_URL}/current`);
        document.getElementById("current").textContent = data.temperature.toFixed(2);
    } catch (e) {
        console.error("Error fetching current temperature", e);
    }
}

async function updateChart() {
    const url = currentPeriod === "hourly" ? "/hourly" : "/daily";
    const labelKey = currentPeriod === "hourly" ? "time" : "date";

    try {
        const data = await fetchJSON(`${API_URL}${url}`);

        historyData = data.map(d => ({
            label: d[labelKey],
            value: d.avg
        }));

        const labels = historyData.map(d => d.label);
        const values = historyData.map(d => d.value);

        const bgColor = currentPeriod === "hourly"
            ? 'rgba(75, 192, 192, 0.2)'
            : 'rgba(255, 159, 64, 0.2)';

        const borderColor = currentPeriod === "hourly"
            ? 'rgba(75, 192, 192, 1)'
            : 'rgba(255, 159, 64, 1)';

        if (chart) {
            chart.data.labels = labels;
            chart.data.datasets[0].data = values;
            chart.data.datasets[0].backgroundColor = bgColor;
            chart.data.datasets[0].borderColor = borderColor;
            chart.update();
        } else {
            const ctx = document.getElementById("chartCanvas").getContext('2d');
            chart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: labels,
                    datasets: [{
                        label: 'Температура °C',
                        data: values,
                        borderColor: borderColor,
                        backgroundColor: bgColor,
                        tension: 0.3,
                        fill: true,
                        pointRadius: 3,
                        pointHoverRadius: 6
                    }]
                },
                options: {
                    responsive: true,
                    plugins: {
                        tooltip: { enabled: true, mode: 'nearest', intersect: false }
                    },
                    scales: {
                        y: { beginAtZero: false }
                    }
                }
            });
        }
    } catch (e) {
        console.error(`Error updating chart for ${currentPeriod}`, e);
    }
}

document.getElementById("btn-hourly").addEventListener("click", () => {
    currentPeriod = "hourly";
    updateChart();
});

document.getElementById("btn-daily").addEventListener("click", () => {
    currentPeriod = "daily";
    updateChart();
});

async function init() {
    await updateCurrent();
    await updateChart();
}


setInterval(async () => {
    await updateCurrent();
    await updateChart();
}, 60000);

/*setInterval(async () => {
    await updateCurrent();
    await updateChart();
}, 1000); для теста обновление каждую секунду*/

init();
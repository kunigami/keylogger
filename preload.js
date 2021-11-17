
const {spawn} = require('child_process');
const dayjs = require('dayjs');
const fs = require('fs');
const Chart = require('chart.js');

const REFRESH_RATE_MS = 60000; // 1 min
const DATA_PATH = process.env.KEYLOGGER_DATA;

window.addEventListener('DOMContentLoaded', () => {
  updatePage();
})

function replaceText (selector, text) {
  const element = document.getElementById(selector);
  if (element) {
    element.innerText = text;
  }
}

function updatePage() {
  const filename = `${DATA_PATH}/histogram_${dayjs().format('YYYY-MM-DD')}.txt`
  try {
    if (!fs.existsSync(filename)) {
      throw new Error(`File ${filename} does not exist.`);
    }
    const time = dayjs().format('YYYY-MM-DDT HH:mm:ss');
    fs.readFile(filename, 'utf8', (err, data) => {
      if (err) {
        throw err;
      }
      const lines = data.split("\n");
      const rows = lines.map(line => {
        const [str, cnt] = line.split(':|:');
        return [str, cnt];
      });

      rows.sort((row1, row2) => {
        return row2[1] - row1[1];
      });


      let total = 0;
      rows.filter(row => !!row[1]).forEach(row => total += parseInt(row[1], 10));

      const output = `Last updated: ${time}\nTotal keystrokes: ${total}`;
      replaceText('output', output);
      renderChart(rows);
    });
  } catch (error) {
    replaceText('output', error.message);
  }
  setTimeout(updatePage, REFRESH_RATE_MS);
}

function renderChart(rows) {
  const ctx = document.getElementById('myChart').getContext('2d');
  const labels = rows.map(row => row[0]);
  const values = rows.map(row => row[1]);

const myChart = new Chart(ctx, {
    type: 'bar',
    options: {
      indexAxis: 'y',
        scales: {
          x: {
            beginAtZero: true
          }
        }
    },

    data: {
        labels: labels,
        datasets: [{
            label: '# Keystrokes',
            data: values,
            backgroundColor: [
                'rgba(54, 162, 235, 0.2)',
            ],
        }]
    },
});
}

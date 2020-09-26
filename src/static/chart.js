let chart = new frappe.Chart("#frost-chart", { // or DOM element
  data: {
    labels: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"],
    datasets: [{
        name: "Dataset 1",
        values: [18, 40, 30, 35, 8, 52, 17, -4]
      },
      {
        name: "Dataset 2",
        values: [30, 50, -10, 15, 18, 32, 27, 14]
      }
    ]
  },

  title: "My Awesome Chart",
  type: 'axis-mixed', // or 'bar', 'line', 'pie', 'percentage'
  height: 300,
  colors: ['purple', '#ffa3ef', 'light-blue'],

  tooltipOptions: {
    formatTooltipX: d => (d + '').toUpperCase(),
    formatTooltipY: d => d + ' pts',
  }
});

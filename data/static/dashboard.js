"use strict";

window.chartColors = {
    red: 'rgb(255, 99, 132)',
    orange: 'rgb(255, 159, 64)',
    yellow: 'rgb(255, 205, 86)',
    green: 'rgb(75, 192, 192)',
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)'
};

var randomScalingFactor = function () {
    return Math.round(Math.random() * 100);
};

var servoDatapoints = [];
var encoderDatapoints = [];
var manualServoDatapoints = [];
var manualEncoderDatapoints = [];

var config = {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Auto - Servo',
            data: servoDatapoints,
            borderColor: window.chartColors.blue,
            backgroundColor: 'rgba(0, 0, 0, 0)',
            fill: true,
        }, {
            label: 'Auto - Rotatry Encoder',
            data: encoderDatapoints,
            borderColor: window.chartColors.green,
            backgroundColor: 'rgba(0, 0, 0, 0)',
            fill: false,
            lineTension: 0
        }, {
            label: 'Manual - Servo',
            data: manualServoDatapoints,
            borderColor: window.chartColors.red,
            backgroundColor: 'rgba(0, 0, 0, 0)',
            fill: false,
        }, {
            label: 'Manual - Rotatry Encoder',
            data: manualEncoderDatapoints,
            borderColor: window.chartColors.yellow,
            backgroundColor: 'rgba(0, 0, 0, 0)',
            fill: false,
            lineTension: 0
        }]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        title: {
            display: true,
            text: 'Serov and Rotary Encoder.'
        },
        tooltips: {
            mode: 'index'
        },
        scales: {
            xAxes: [{
                display: true,
                scaleLabel: {
                    display: true
                }
            }],
            yAxes: [{
                display: true,
                scaleLabel: {
                    display: true,
                    labelString: 'Value'
                },
                ticks: {
                    suggestedMin: 0,
                    suggestedMax: 200,
                }
            }]
        }
    }
};

window.onload = function () {
    var ctx = document.getElementById('myChart').getContext('2d');
    window.myLine = new Chart(ctx, config);
};

document.getElementById('manualReading').addEventListener('click', function () {
    $("#manualReadingModal").modal();
});

document.getElementById('takeManualReading').addEventListener('click', function () {
    var servoAngle = document.getElementById('servoAngle').value;
    $.getJSON("http://192.168.43.199/manualTestData?servoAngle=" + servoAngle, function (data) {
        var index = document.getElementById('manualTbody').children.length + 1;
        var eachrow = "<tr>"
            + "<td>" + index + "</td>"
            + "<td>" + servoAngle + "</td>"
            + "<td>" + data["e"] + "</td>"
            + "</tr>";
        $('#manualTbody').append(eachrow);
        if (index > config.data.labels.length) {
            config.data.labels.push(config.data.labels.length);
        }
        manualServoDatapoints.push(servoAngle);
        manualEncoderDatapoints.push(data["e"]);
        window.myLine.update();
        $("#manualReadingModal").modal("hide");
    });
});

document.getElementById('autoReading').addEventListener('click', function () {
    $("#loadingModal").modal();
    $.getJSON("http://192.168.43.199/autoTestData", function (data) {
        $('#autoTbody').empty();
        encoderDatapoints.length = 0;
        servoDatapoints.length = 0;
        $.each(data, function (index, item) {
            var rowNumber = parseInt(index) + 1; 
            var eachrow = "<tr>"
                + "<td>" + rowNumber + "</td>"
                + "<td>" + item["s"] + "</td>"
                + "<td>" + item["e"] + "</td>"
                + "</tr>";
            $('#autoTbody').append(eachrow);
            if (config.data.labels.length <= 30) {
                config.data.labels.push(config.data.labels.length);
            }
            servoDatapoints.push(item["s"]);
            encoderDatapoints.push(item["e"]);
        });
        window.myLine.update();
        $("#loadingModal").modal("hide");
    });
});


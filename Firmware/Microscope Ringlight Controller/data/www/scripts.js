var ledstate = false;
var brightness = 0;
var temperature = 5000;
var direction = 0;

window.onload = function() {
    // Get status on page load
    getStatus();

    // Then set up to call every 2 seconds
    setInterval(getStatus, 2000);
};

function toggleLED(state) {
    ledstate = state ? 1:0;
    updatePowerButtons(state);
    const data = { power: ledstate };
    fetch('/power', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(data),
    })
        .then(response => response.json())
        .then(data => {
            console.log('Success:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
}

function brightnessChange(val){
    if (val >=0 && val < 256){
        brightness = val;
        document.getElementById('BrightnessLabel').innerHTML = val;
        const data = { brightness: brightness };
        fetch('/brightness', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
            .then(response => response.json())
            .then(data => {
                console.log('Success:', data);
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }
}

function temperatureChange(val){
    if (val >=700 && val < 40000){
        temperature = val;
        document.getElementById('TemperatureLabel').innerHTML = val;
        const data = { temperature: temperature };
        fetch('/temperature', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
            .then(response => response.json())
            .then(data => {
                console.log('Success:', data);
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }
}

function directionChange(val){
    if (val >=0 && val < 27){
        direction = val;
        document.getElementById('DirectionLabel').innerHTML = val;
        const data = { direction: direction };
        fetch('/direction', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
            .then(response => response.json())
            .then(data => {
                console.log('Success:', data);
            })
            .catch((error) => {
                console.error('Error:', error);
            });
    }
}

async function getStatus() {
    try {
        const response = await fetch('/getstate');
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }

        const data = await response.json();

        if (data.lights && data.lights.length > 0) {
            const light = data.lights[0];

            document.getElementById('brightness').value = light.brightness;
            document.getElementById('temperature').value = light.temperature;
            document.getElementById('direction').value = light.direction;

            document.getElementById('BrightnessLabel').innerHTML = light.brightness;
            document.getElementById('TemperatureLabel').innerHTML = light.temperature;
            document.getElementById('DirectionLabel').innerHTML = light.direction;

            document.getElementById('direction').max = data.numberOfLights;

            if (light.on === 1) {
                updatePowerButtons(true);
            }
            else{
                updatePowerButtons(false);
            }
        }
    } catch (error) {
        console.error('Failed to fetch light state:', error);
    }
}

function updatePowerButtons(state) {
    if (state) {
        document.getElementById('PowerLabel').innerHTML = "On";
        document.getElementById("onbutton").setAttribute("style", "background-color:green");
        document.getElementById("offbutton").setAttribute("style", "background-color:grey");
    }
    else{
        document.getElementById('PowerLabel').innerHTML = "Off";
        document.getElementById("onbutton").setAttribute("style", "background-color:grey");
        document.getElementById("offbutton").setAttribute("style", "background-color:green");
    }
}

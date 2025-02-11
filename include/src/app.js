class Room {
    constructor(id, name, currentTemperature, targetTemperature, forced) {
        this.id = id;
        this.name = name;
        this.currentTemperature = currentTemperature;
        this.targetTemperature = targetTemperature;
        this.forced = forced;
        this.element = this.createElement();
    }

    createElement() {
        const card = document.createElement('div');
        card.classList.add('thermostat-card');
        card.dataset.id = this.id;

        const backButton = document.createElement('button');
        backButton.classList.add('back-button');
        backButton.textContent = '←';
        backButton.addEventListener('click', (e) => {
            e.stopPropagation();
            this.closeCard();
        });

        const title = document.createElement('h2');
        title.textContent = this.name;

        const temperatures = document.createElement('div');
        temperatures.classList.add('temperatures');

        const currentTemp = document.createElement('div');
        currentTemp.classList.add('temperature');
        currentTemp.innerHTML = `<div class="temperature-value">${this.currentTemperature.toFixed(1)}°</div><div class="temperature-label">Current</div>`;

        const targetTemp = document.createElement('div');
        targetTemp.classList.add('temperature');
        targetTemp.innerHTML = `<div class="temperature-value">${this.targetTemperature.toFixed(1)}°</div><div class="temperature-label">Target</div>`;

        temperatures.appendChild(currentTemp);
        temperatures.appendChild(targetTemp);

        const quickActions = document.createElement('div');
        quickActions.classList.add('quick-actions');

        const ecoButton = document.createElement('button');
        ecoButton.classList.add('action-button');
        ecoButton.textContent = 'Eco';
        ecoButton.addEventListener('click', () => this.sendCommand('eco'));

        const boostButton = document.createElement('button');
        boostButton.classList.add('action-button');
        boostButton.textContent = 'Boost';
        boostButton.addEventListener('click', () => this.sendCommand('boost'));

        quickActions.appendChild(ecoButton);
        quickActions.appendChild(boostButton);

        const controls = document.createElement('div');
        controls.classList.add('controls');

        const slider = document.createElement('input');
        slider.type = 'range';
        slider.classList.add('temperature-slider');
        slider.min = 5;
        slider.max = 30;
        slider.step = 0.5;
        slider.value = this.targetTemperature;
        slider.addEventListener('input', (e) => {
            targetTemp.querySelector('.temperature-value').textContent = `${parseFloat(e.target.value).toFixed(1)}°`;
            this.targetTemperature = parseFloat(e.target.value).toFixed(1);
            this.updateHeatingIndicator();
            
        });
        slider.addEventListener('mouseup', (e) => {
            this.sendCommand('act_temperature');

        } );

        controls.appendChild(slider);

        card.appendChild(backButton);
        card.appendChild(title);
        card.appendChild(temperatures);
        card.appendChild(quickActions);
        card.appendChild(controls);

        this.element = card; // Ensure this.element is assigned before calling updateHeatingIndicator
        this.updateHeatingIndicator();

        card.addEventListener('click', (e) => {
            if (e.target.closest('.action-button') || e.target === backButton) return;
            if (!card.classList.contains('expanded')) {
                document.querySelectorAll('.thermostat-card.expanded').forEach(expandedCard => {
                    expandedCard.classList.remove('expanded');
                });
                card.classList.add('expanded');
                document.body.style.overflow = 'hidden';
            }
        });

        return card;
    }

    updateHeatingIndicator() {
        if (this.targetTemperature > this.currentTemperature) {
            this.element.dataset.heating = "true";
            if (!this.element.querySelector('.heating-indicator')) {
                const indicator = document.createElement('div');
                indicator.classList.add('heating-indicator');
                this.element.appendChild(indicator);
            }
        } else {
            this.element.dataset.heating = "false";
            const indicator = this.element.querySelector('.heating-indicator');
            if (indicator) {
                indicator.remove();
            }
        }
    }

    closeCard() {
        this.element.classList.remove('expanded');
        document.body.style.overflow = 'auto';
    }

    sendCommand(command) {
        const message = {
            id: this.id,
            command: command,
            targetTemperature: this.targetTemperature,
            forced: this.forced
        };
        ws.send(JSON.stringify(message));
    }

    update(data) {
        this.currentTemperature = data.currentTemperature;
        this.targetTemperature = data.targetTemperature;
        this.forced = data.forced;
        this.element.querySelector('.temperature:nth-child(1) .temperature-value').textContent = `${this.currentTemperature.toFixed(1)}°`;
        this.element.querySelector('.temperature:nth-child(2) .temperature-value').textContent = `${this.targetTemperature.toFixed(1)}°`;
        this.updateHeatingIndicator();
    }
}

let rooms = {};

function handleWebSocketMessage(data) {
    const parsedData = JSON.parse(data);
    console.log('Received message:', data);
    const thermostatList = document.getElementById('thermostat-list');
    const loader = document.getElementById('loader');
    if (parsedData.rooms) {
        const roomData = parsedData.rooms;
        roomData.forEach(room => {
            if (rooms[room.id]) {
                rooms[room.id].update(room);
            } else {
                const newRoom = new Room(room.id, room.name, room.currentTemperature, room.targetTemperature, room.forced);
                rooms[room.id] = newRoom;
                if (thermostatList) {
                    thermostatList.appendChild(newRoom.element);
                } else {
                    console.error('Element with ID "thermostat-list" not found');
                }
            }
        });
        if (thermostatList.children.length > 0) {
            loader.style.display = 'none';
        }
    } else {
        console.log('Received message:', data);
    }
}

let ws;
const wsStatusElement = document.getElementById('ws-status');

function connectWebSocket() {
    ws = new WebSocket('ws://192.168.8.43:81');

    ws.onopen = function(event) {
        console.log('WebSocket connection opened:', event);
        wsStatusElement.classList.add('connected');
        wsStatusElement.querySelector('span').textContent = 'WS Connected';
        // Send a message to the server
        ws.send(JSON.stringify({ message: "Hello Server!" }));
    };

    ws.onmessage = function(event) {
        console.log('Message from server:', event.data);
        handleWebSocketMessage(event.data);
    };

    ws.onclose = function(event) {
        console.log('WebSocket connection closed:', event);
        wsStatusElement.classList.remove('connected');
        // wsStatusElement.querySelector('span').textContent = 'WS Disconnected';
        // Ponowne połączenie po 5 sekundach
        setTimeout(connectWebSocket, 500);
    };

    ws.onerror = function(error) {
        console.error('WebSocket error:', error);
    };
}

// Nawiązanie połączenia WebSocket przy całkowitym załadowaniu strony



document.addEventListener("DOMContentLoaded", function () {
    
    connectWebSocket();
    console.log('DOM fully loaded and parsed');
    
    
    const configButton = document.querySelector(".noselect");
    const wifiConfigButton = document.querySelector("#wifiConfig");
    const modal = document.getElementById("configModal");
    const span = document.getElementsByClassName("close")[0];
    const saveButton = document.getElementById("saveConfig");
    const roomList = document.getElementById("roomList");



  /*   socket.onmessage = function (event) {
      const data = JSON.parse(event.data);
      if (data.rooms) {
        roomList.innerHTML = "";
        data.rooms.forEach((room) => {
          const roomItem = document.createElement("div");
          roomItem.innerHTML = `
            <label>${room.name}</label>
            <select data-room-id="${room.id}">
              <option value="1">Pin 1</option>
              <option value="2">Pin 2</option>
              <option value="3">Pin 3</option>
              <option value="4">Pin 4</option>
            </select>
          `;
          roomList.appendChild(roomItem);
        });
      }
    }; */

    configButton.onclick = function () {
      modal.style.display = "block";
    //   socket.send(JSON.stringify({ command: "getRooms" }));
    };
    wifiConfigButton.onclick = function () {
        modal.style.display = "block";
        document.location.href = "/config";

      };

    span.onclick = function () {
      modal.style.display = "none";
    };

    window.onclick = function (event) {
      if (event.target == modal) {
        modal.style.display = "none";
      }
    };

    saveButton.onclick = function () {
      const roomConfigs = [];
      const selects = roomList.querySelectorAll("select");
      selects.forEach((select) => {
        roomConfigs.push({
          id: select.getAttribute("data-room-id"),
          pinNumber: select.value,
        });
      });
      socket.send(JSON.stringify({ command: "updatePins", rooms: roomConfigs }));
      modal.style.display = "none";
    };


    
  });
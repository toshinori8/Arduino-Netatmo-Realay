class Room {
   /**
   * @param {string} id
   * @param {string} name
   * @param {number} currentTemperature
   * @param {number} targetTemperature
   * @param {boolean} forced
   * @param {string} battery_level
   * @param {string} battery_state
   * @param {number} priority
   * @param {boolean} valve
   * @param {number} pinNumber
   */
  constructor(
    id,
    name,
    currentTemperature,
    targetTemperature,
    forced,
    battery_level,
    battery_state,
    priority = 0,
    valve = false,
    pinNumber = 0
  ) {
    this.id = id;
    this.name = name;
    this.currentTemperature = currentTemperature;
    this.targetTemperatureNetatmo = targetTemperature; // Renamed for clarity
    this.targetTemperatureFireplace = targetTemperature; // Initialize fireplace temp, maybe update later if different
    this.forced = forced;
    this.battery_state = battery_state;
    this.battery_level = battery_level;
    this.priority = priority;
    this.valve = valve;
    this.pinNumber = pinNumber;
    this.element = this.createElement();
  }

  createElement() {
    // Dodajemy style dla valve-indicator jeśli jeszcze nie istnieją
    if (!document.getElementById('valve-indicator-style')) {
      const style = document.createElement('style');
      style.id = 'valve-indicator-style';
      document.head.appendChild(style);
    }

    const card = document.createElement("div");
    card.classList.add("thermostat-card");
    card.dataset.id = this.id;
    card.dataset.pin = this.pinNumber;

    const backButton = document.createElement("button");
    backButton.classList.add("back-button");
    backButton.textContent = "←";
    backButton.addEventListener("click", (e) => {
      e.stopPropagation();
      this.closeCard();
    });

    const title = document.createElement("h2");
    title.textContent = this.name;

    const temperatures = document.createElement("div");
    temperatures.classList.add("temperatures");

    const currentTemp = document.createElement("div");
    currentTemp.classList.add("temperature");
    currentTemp.innerHTML = `<div class="temperature-value">${this.currentTemperature.toFixed(
      1
    )}°</div><div class="temperature-label">Aktualna</div>`; // Polish label

    const targetTempNetatmo = document.createElement("div");
    targetTempNetatmo.classList.add("temperature");
    targetTempNetatmo.innerHTML = `<div class="temperature-value">${this.targetTemperatureNetatmo.toFixed(
      1
    )}°</div><div class="temperature-label">Cel Netatmo</div>`; // Slider 1 Target

    const targetTempFireplace = document.createElement("div");
    targetTempFireplace.classList.add("temperature");
    targetTempFireplace.innerHTML = `<div class="temperature-value">${this.targetTemperatureFireplace.toFixed(
      1 // Initialize with fireplace temp
    )}°</div><div class="temperature-label">Cel Kominek</div>`; // Slider 2 Target

    temperatures.appendChild(currentTemp);
    temperatures.appendChild(targetTempNetatmo);
    temperatures.appendChild(targetTempFireplace); // Add fireplace temp display

    const quickActions = document.createElement("div");
    quickActions.classList.add("quick-actions");

    // Add fire button
    const fireButton = document.createElement("button");
    fireButton.classList.add("action-button", "a_fire");
    if (this.forced) {
      fireButton.classList.add("active");
    }
    fireButton.addEventListener("click", () => {
      const isActive = !fireButton.classList.contains("active");
      console.log(isActive);
      this.forced = isActive;
      this.sendCommand("forced", isActive);
      fireButton.classList.toggle("active");
    });

    // skopiuj ikonke svg a_fire
    /* const fireButtonSvgCopy = document
      .querySelector("footer .a_fire")
      .cloneNode(true); */

    // fireButton.classList.add("action-button", "a_fire");
    // fireButton.appendChild(fireButtonSvgCopy);
    // console.log(fireButtonSvgCopy);



    const ecoButton = document.createElement("button");
    ecoButton.classList.add("action-button");
    ecoButton.classList.add("eco-button");
    ecoButton.textContent = "Eco";
    ecoButton.addEventListener("click", () => this.sendCommand("eco"));

    const boostButton = document.createElement("button");
    boostButton.classList.add("action-button");
    boostButton.textContent = "Boost";
    boostButton.addEventListener("click", () => this.sendCommand("boost"));

    quickActions.appendChild(fireButton);
    quickActions.appendChild(ecoButton);
    // quickActions.appendChild(boostButton);


    const controls = document.createElement("div");
    controls.classList.add("controls");

    // --- Slider 1: Netatmo Target ---
    const sliderNetatmoLabel = document.createElement("label");
    sliderNetatmoLabel.textContent = "Cel Netatmo:";
    sliderNetatmoLabel.classList.add("slider-label");

    const sliderNetatmo = document.createElement("input");
    sliderNetatmo.type = "range";
    sliderNetatmo.classList.add("temperature-slider", "netatmo-slider");
    sliderNetatmo.min = 5; // Min temp for Netatmo
    sliderNetatmo.max = 30;
    sliderNetatmo.step = 0.5; // Step for Netatmo
    sliderNetatmo.value = this.targetTemperatureNetatmo;
    sliderNetatmo.addEventListener("input", (e) => {
      targetTempNetatmo.querySelector(
        ".temperature-value"
      ).textContent = `${parseFloat(e.target.value).toFixed(1)}°`;
      // Update internal value but don't send command yet
      this.targetTemperatureNetatmo = parseFloat(e.target.value);
      this.updateHeatingIndicator(); // Update indicator immediately
    });
    sliderNetatmo.addEventListener("mouseup", () => {
      // Send command only when user releases slider
      this.sendCommand("act_temperature");
    });

    // --- Slider 2: Fireplace Target ---
    const sliderFireplaceLabel = document.createElement("label");
    sliderFireplaceLabel.textContent = "Cel Kominek:";
    sliderFireplaceLabel.classList.add("slider-label");

    const sliderFireplace = document.createElement("input");
    sliderFireplace.type = "range";
    sliderFireplace.classList.add("temperature-slider", "fireplace-slider");
    sliderFireplace.min = 10; // Different range/step for fireplace?
    sliderFireplace.max = 28;
    sliderFireplace.step = 0.5;
    sliderFireplace.value = this.targetTemperatureFireplace;
    sliderFireplace.addEventListener("input", (e) => {
      targetTempFireplace.querySelector(
        ".temperature-value"
      ).textContent = `${parseFloat(e.target.value).toFixed(1)}°`;
      // Update internal value
      this.targetTemperatureFireplace = parseFloat(e.target.value);
      this.updateHeatingIndicator(); // Update indicator immediately
    });
    sliderFireplace.addEventListener("mouseup", () => {
      // Send specific command for fireplace target
      this.sendFireplaceCommand();
    });

    controls.appendChild(sliderNetatmoLabel);
    controls.appendChild(sliderNetatmo);
    controls.appendChild(sliderFireplaceLabel);
    controls.appendChild(sliderFireplace);

    const batteryIndicator = this.createBatteryIndicator();

    // Create priority display
    const priorityDisplay = document.createElement("p");
    priorityDisplay.classList.add("priority-display");
    priorityDisplay.textContent = `Priority: ${this.priority}`;
    priorityDisplay.style.fontSize = "20px";
    priorityDisplay.style.color = "#666";
    priorityDisplay.style.marginTop = "-28px";
    priorityDisplay.style.textAlign = "center";

    card.appendChild(backButton);
    card.appendChild(title);
    card.appendChild(temperatures);
    card.appendChild(quickActions);
    card.appendChild(controls);
    card.appendChild(priorityDisplay);
    card.appendChild(batteryIndicator);

    this.element = card; // Ensure this.element is assigned before calling updateHeatingIndicator
    this.updateHeatingIndicator();
    this.updateValveIndicator();


    card.addEventListener("click", (e) => {
      if (e.target.closest(".action-button") || e.target === backButton) return;
      if (card.classList.contains("expanded")) {
        // Jeśli karta jest już rozwinięta, usuń klasę expanded
        card.classList.remove("expanded");
        document.body.style.overflow = "auto";
      } else {
        // Jeśli karta nie jest rozwinięta, rozwiń ją
        document
          .querySelectorAll(".thermostat-card.expanded")
          .forEach((expandedCard) => {
            expandedCard.classList.remove("expanded");
          });
        card.classList.add("expanded");
       // document.body.style.overflow = "hidden";
      }
    });
    return card;
  }

  createBatteryIndicator() {
    const batteryIndicator = document.createElement("div");
    batteryIndicator.classList.add("battery-indicator");

    const batterySvg = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "svg"
    );
    batterySvg.setAttribute("class", "battery_indicator");
    batterySvg.setAttribute("viewBox", "0 0 167.64 101.93");

    const defs = document.createElementNS("http://www.w3.org/2000/svg", "defs");
    const style = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "style"
    );
    style.textContent = `
     .ramka path, .ramka2 path, .bars path { 
          opacity: 1;
          }

      .bt1 { fill: #FF2F00; }
    @keyframes pulse {
        0% {
            fill-opacity: 1;
        }
        50% {
            fill-opacity: 0.5;
        }
        100% {
            fill-opacity: 1;
        }
    }

    .pulsing {
        animation: pulse 1s infinite;
    }
      .bt2 { fill: #FFAA00; }
   
      .bt4 { fill: #00FF00; }
      .hidden { display: none; }
    `;
    defs.appendChild(style);
    batterySvg.appendChild(defs);

    const ramka = document.createElementNS("http://www.w3.org/2000/svg", "g");
    ramka.setAttribute("class", "ramka");
    const ramkaPath = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "path"
    );
    ramkaPath.setAttribute("class", "");
    ramkaPath.setAttribute(
      "d",
      "M75.57,101.92c-20.66,0-41.33.01-61.99,0-9.24,0-13.56-4.2-13.57-13.33C-.01,63.42,0,38.26.07,13.09.09,4.14,4.33.01,13.3,0,54.79,0,96.29,0,137.78.01c8.72,0,12.9,4.17,12.9,12.8.01,25.33.01,50.66,0,75.99,0,8.96-4.15,13.1-13.11,13.11-20.66.02-41.33,0-61.99,0ZM76.07,10.08c-20.15,0-40.3.15-60.44-.12-4.59-.06-5.49,1.44-5.45,5.66.2,23.64.22,47.29.06,70.93-.03,4.07.97,5.36,5.22,5.35,40.12-.16,80.25-.16,120.38,0,4.15.02,4.86-1.44,4.84-5.15-.14-23.81-.17-47.62.02-71.43.03-4.06-.92-5.38-5.18-5.33-19.81.23-39.63.1-59.44.09Z"
    );
    ramka.appendChild(ramkaPath);
    batterySvg.appendChild(ramka);

    const ramka2 = document.createElementNS("http://www.w3.org/2000/svg", "g");
    ramka2.setAttribute("class", "ramka2");
    const ramkaPath2 = document.createElementNS(
      "http://www.w3.org/2000/svg",
      "path"
    );
    ramkaPath2.setAttribute("class", "");
    ramkaPath2.setAttribute(
      "d",
      "M153.84,50.84c0-3.99-.07-7.98.01-11.97.09-4.69,2.39-6.32,6.79-4.61,4.25,1.65,6.65,4.98,6.84,9.53.2,4.81.24,9.65-.03,14.45-.27,4.74-2.85,8.1-7.44,9.53-4.07,1.27-6-.16-6.13-4.48-.13-4.15-.03-8.31-.03-12.46Z"
    );
    ramka2.appendChild(ramkaPath2);
    batterySvg.appendChild(ramka2);

    const bars = document.createElementNS("http://www.w3.org/2000/svg", "g");
    bars.setAttribute("class", "bars");

    const batteryLevels = [
      {
        class: "hidden",
        d: "M34.61,51.48c0,9.66-.11,19.31.05,28.97.07,4.11-1.63,5.68-5.63,5.6-15.29-.31-13.81,2.03-13.85-13.36-.04-16.81.12-33.63-.06-50.44-.05-4.78,1.67-6.47,6.44-6.39,14.69.24,12.99-1.36,13.06,13.15.02,3.83.01,7.66.01,11.49,0,3.66,0,7.32,0,10.99h-.02Z",
      },
      {
        class: "hidden",
        d: "M134.57,51.28c0,9.67-.1,19.33.05,28.99.06,4.01-1.49,5.82-5.56,5.75-14.88-.26-14.28,2.24-14.24-13.66.04-16.5,0-32.99.02-49.49,0-6.18.82-7.01,6.86-6.95,13.8.14,12.94-1.76,12.88,12.86-.03,7.5,0,15,0,22.5Z",
      },
      {
        class: "hidden",
        d: "M109.57,51.53c0,9.64-.09,19.28.04,28.92.05,4.03-1.54,5.68-5.62,5.6-14.8-.28-14.16,2.11-14.1-13.74.06-16.29,0-32.57.02-48.86,0-6.92.56-7.45,7.23-7.5,13.72-.1,12.43-1.23,12.43,12.65,0,7.64,0,15.29,0,22.93Z",
      },
      {
        class: "hidden",
        d: "M64.96,50.5c0-9.49.08-18.99-.04-28.48-.06-4.42,1.66-6.22,6.24-6.15,15.2.23,13.37-1.39,13.4,13.3.04,16.82-.11,33.64.09,50.46.06,4.78-1.63,6.57-6.4,6.44-13.91-.4-13.24,2.35-13.28-13.59-.02-7.33,0-14.65-.01-21.98Z",
      },
      {
        class: "hidden",
        d: "M59.59,51.1c0,9.82-.09,19.65.04,29.47.05,3.81-1.47,5.5-5.28,5.45-15.79-.21-14.17,2.01-14.24-13.49-.08-16.65-.04-33.3,0-49.96.01-5.95.71-6.64,6.55-6.63,14.35.02,12.92-1.49,12.94,12.68,0,7.49,0,14.99,0,22.48Z",
      },
    ];

    batteryLevels.forEach((level) => {
      const path = document.createElementNS(
        "http://www.w3.org/2000/svg",
        "path"
      );
      path.setAttribute("class", `${level.class} hidden`);
      path.setAttribute("d", level.d);
      bars.appendChild(path);
    });

    batterySvg.appendChild(bars);
    batteryIndicator.appendChild(batterySvg);

    this.updateBatteryIndicator(batterySvg);

    return batteryIndicator;
  }

  updateBatteryIndicator(batterySvg) {
    const battery_state = this.battery_state;
    /* const battery_level = this.battery_level; */
    const ramka = batterySvg.querySelector(".ramka path");
    const ramka2 = batterySvg.querySelector(".ramka2 path");
    const levels = batterySvg.querySelectorAll(".bars path");

    // Reset all levels to hidden
    levels.forEach((level) => level.classList.add("hidden"));

    if (battery_state === "full") {
      ramka.setAttribute("fill", "#00ff00");
      ramka2.setAttribute("fill", "#00ff00");
      levels.forEach((level) => {
        levels[0].classList.remove("hidden");
        levels[1].classList.remove("hidden");
        levels[2].classList.remove("hidden");
        levels[3].classList.remove("hidden");
        levels[4].classList.remove("hidden");
        level.setAttribute("fill", "#00ff00");
      });
    } else if (battery_state === "high") {
      ramka.setAttribute("fill", "#FFAA00");
      ramka2.setAttribute("fill", "#FFAA00");
      levels.forEach((level) => {
        levels[0].classList.remove("hidden");
        levels[3].classList.remove("hidden");
        levels[4].classList.remove("hidden");

        level.setAttribute("fill", "#FFAA00");
      });
    } else if (battery_state === "low") {
      ramka.setAttribute("fill", "#FF2F00");
      ramka2.setAttribute("fill", "#FF2F00");
      levels[0].classList.remove("hidden");
      levels[3].classList.remove("hidden");
      level.setAttribute("fill", "#FF2F00");
    } else if (battery_state === "very_low") {
      ramka.classList.add("pulsing");
      ramka2.classList.add("pulsing");
      ramka.setAttribute("fill", "#FF2F00");
      ramka2.setAttribute("fill", "#FF2F00");
      levels[0].classList.remove("hidden");
      level.setAttribute("fill", "#FF2F00");
      // dodaj do kazdego levels klase pulsing
      levels.forEach((level) => {
        level.classList.add("pulsing");
      });
    }
  }

  updateHeatingIndicator() {
    // Use global useGazState (defined later)
    let effectiveTarget;
    if (window.useGazState === true) { // Check global state
        effectiveTarget = Math.max(this.targetTemperatureNetatmo, this.targetTemperatureFireplace);
    } else {
        effectiveTarget = this.targetTemperatureFireplace;
    }

    if (this.currentTemperature < effectiveTarget) {
      this.element.dataset.heating = "true"; // Used for body class
      if (!this.element.querySelector(".heating-indicator")) {
        const indicator = document.createElement("div");
        indicator.classList.add("heating-indicator");
        this.element.appendChild(indicator);
      }
    } else {
      this.element.dataset.heating = "false";
      const indicator = this.element.querySelector(".heating-indicator");
      if (indicator) {
        indicator.remove();
      }
    }
  }

  updateValveIndicator() {
    if (this.valve) {
      this.element.dataset.valve = "true";
      if (!this.element.querySelector(".valve-indicator")) {
        const indicator = document.createElement("div");
        indicator.classList.add("valve-indicator");
        this.element.appendChild(indicator);
      }
    } else {
      this.element.dataset.valve = "false";
      const indicator = this.element.querySelector(".valve-indicator");
      if (indicator) {
        indicator.remove();
      }
    }
  }

  closeCard() {
    this.element.classList.remove("expanded");
    document.body.style.overflow = "auto";
  }

  // Command for Slider 1 (Netatmo)
  sendCommand(command) {
    const message = {
      id: this.id,
      command: command, // e.g., "act_temperature", "forced"
      targetTemperature: parseFloat(this.targetTemperatureNetatmo).toFixed(1), // Send Netatmo target
      forced: this.forced,
      // priority: this.priority // Priority is calculated backend
    };

    console.log(message);
    console.log("Sending command:", message);
    ws.send(JSON.stringify(message));
  }

  // Command for Slider 2 (Fireplace)
  sendFireplaceCommand() {
      const message = {
          id: this.id,
          command: "set_fireplace_target", // New command
          targetTemperatureFireplace: parseFloat(this.targetTemperatureFireplace).toFixed(1) // Send fireplace target
      };
      console.log("Sending fireplace command:", message);
      ws.send(JSON.stringify(message));
  }
  sendMinimalTemperature(){
    temp= document.querySelector("#configModal .manifoldMinTemp").value;
    if(temp != ""){
       ws.send(JSON.stringify({ command: "manifoldMinTemp", value: temp}));
    }


  }


  update(data) {
    this.currentTemperature = data.currentTemperature;
    // Update both target temperatures from backend data
    this.targetTemperatureNetatmo = data.targetTemperatureNetatmo;
    this.targetTemperatureFireplace = data.targetTemperatureFireplace;
    this.forced = data.forced;
    this.battery_state = data.battery_state;
    this.priority = data.priority || this.priority; // Update priority if available
    this.valve = data.valve || false; // Update valve status if available
    this.pinNumber = data.pinNumber || this.pinNumber; // Update pin number if available

    // Update UI elements
    this.element.querySelector( // Current Temp
      ".temperatures .temperature:nth-child(1) .temperature-value"
    ).textContent = `${this.currentTemperature.toFixed(1)}°`;
    this.element.querySelector( // Netatmo Target Display
      ".temperatures .temperature:nth-child(2) .temperature-value"
    ).textContent = `${this.targetTemperatureNetatmo.toFixed(1)}°`;
     this.element.querySelector( // Fireplace Target Display
      ".temperatures .temperature:nth-child(3) .temperature-value"
    ).textContent = `${this.targetTemperatureFireplace.toFixed(1)}°`;

    // Update slider positions
    const netatmoSlider = this.element.querySelector(".netatmo-slider");
    if (netatmoSlider) netatmoSlider.value = this.targetTemperatureNetatmo;
    const fireplaceSlider = this.element.querySelector(".fireplace-slider");
    if (fireplaceSlider) fireplaceSlider.value = this.targetTemperatureFireplace;

     // Update forced button state
    const fireButton = this.element.querySelector(".action-button.a_fire");
    if (fireButton) {
        if (this.forced) {
            fireButton.classList.add("active");
        } else {
            fireButton.classList.remove("active");
        }
    }

    // Update priority display
    const priorityDisplay = this.element.querySelector(".priority-display");
    if (priorityDisplay) {
      priorityDisplay.textContent = `Priority: ${this.priority}`;
    }

    // Update data attributes
    this.element.dataset.pin = this.pinNumber;
    this.element.dataset.id = this.id;
    this.element.dataset.forced = this.forced;

    this.updateHeatingIndicator();
    this.updateValveIndicator();
    this.updateBatteryIndicator(
      this.element.querySelector(".battery_indicator")
    );
  }
}

let rooms = {};
window.useGazState = false; // Global state for useGaz

function updateBodyHeatingClass() {
  const thermostatList = document.getElementById("thermostat-list");
  if (!thermostatList) return; // Guard against null
  const thermostatCards =
    thermostatList.getElementsByClassName("thermostat-card");
  let isHeating = false;
  
  for (let card of thermostatCards) {
    // Check the data attribute set by updateHeatingIndicator
    // if heating and forced are both true, it's considered heating
    if (card.dataset.heating === "true"  && card.dataset.forced === "true") {
      isHeating = true;
      break;
    }
  }

  if (isHeating) {
    document.body.classList.add("heating");
  } else {
    document.body.classList.remove("heating");
  }
}

function handleWebSocketMessage(data) {
  const parsedData = JSON.parse(data);
  console.log("Received parsed data:", parsedData); // Log received data

  const thermostatList = document.getElementById("thermostat-list");
  const loader = document.getElementById("loader");

  // Check for meta field first to update global state
   if (parsedData.meta && parsedData.meta.usegaz !== undefined) {
        window.useGazState = parsedData.meta.usegaz === "true";
        const useGazButton = document.getElementById("useGazButton");
        if (useGazButton) {
             if (window.useGazState) {
                useGazButton.classList.add("active");
             } else {
                 useGazButton.classList.remove("active");
             }
        }
         console.log("Updated useGazState:", window.useGazState);
    }

  if(parsedData.meta && parsedData.meta.manifoldTemp !== undefined){
        
        el = document.querySelector("footer .a_fire .manifoldTemp");
        manifoldTemperature = parsedData.meta.manifoldTemp;
        el.innerHTML = manifoldTemperature + "°C";
  }
  if(parsedData.meta && parsedData.meta.manifoldMinTemp !== undefined){
        console.log("Min operating temperature:", parsedData.meta.manifoldMinTemp);
        document.querySelector("#configModal .manifoldMinTemp").value = Number(parsedData.meta.manifoldMinTemp);
      
  
  }
  if (parsedData.rooms) {
    const roomData = parsedData.rooms;
    roomData.forEach((room) => {
      // Make sure both target temperatures exist in the data
      const targetNetatmo = room.targetTemperatureNetatmo !== undefined ? room.targetTemperatureNetatmo : 18.0; // Default if missing
      const targetFireplace = room.targetTemperatureFireplace !== undefined ? room.targetTemperatureFireplace : 18.0; // Default if missing

      if (rooms[room.id]) {
         // Pass both targets to the update method
         rooms[room.id].update({
            ...room, // Spread other properties
            targetTemperatureNetatmo: targetNetatmo,
            targetTemperatureFireplace: targetFireplace
         });
      } else {
        const newRoom = new Room(
          room.id,
          room.name,
          room.currentTemperature,
          targetNetatmo, // Pass Netatmo target to constructor
          room.forced,
          room.battery_level,
          room.battery_state,
          room.priority,
          room.valve,
          room.pinNumber
        );
        // Set fireplace temp separately after creation if needed, or modify constructor
        newRoom.targetTemperatureFireplace = targetFireplace;

        rooms[room.id] = newRoom;
        if (thermostatList) {
          thermostatList.appendChild(newRoom.element);
        } else {
          console.error('Element with ID "thermostat-list" not found');
        }
      }
    });
    if (thermostatList && thermostatList.children.length > 0) {
      loader.style.display = "none";
    }
     // Update indicators for all rooms after processing batch, as useGazState might have changed
     Object.values(rooms).forEach(room => room.updateHeatingIndicator());

  } else if (parsedData.command === "getRooms") { // Assuming this is for config modal
    const roomList = document.getElementById("roomList");
    if (!roomList) return;
    roomList.innerHTML = "";

    parsedData.rooms.forEach(room => {
      const div = document.createElement("div");
      div.className = "room-item";
      div.innerHTML = `
        <input type="text" class="room-name" value="${room.name}" data-room-id="${room.id}" placeholder="Nazwa pokoju">
        <select class="room-pin" data-room-id="${room.id}">
          ${Array.from({length: 54}, (_, i) =>
            `<option value="${i}" ${room.pin === i ? 'selected' : ''}">Pin ${i}</option>`
          ).join('')}
        </select>
      `;
      roomList.appendChild(div);
    });
  } else if (parsedData.pinMappings) {
    // Aktualizuj listę w modalu
    const roomList = document.getElementById("roomList");
    roomList.innerHTML = "";

    parsedData.pinMappings.forEach(mapping => {
      const div = document.createElement("div");
      div.className = "room-item";
      div.innerHTML = `
        <input type="text" class="room-name" value="${mapping.name}" data-room-id="${mapping.roomId}" placeholder="Nazwa pokoju">
        <select class="room-pin" data-room-id="${mapping.roomId}">
          ${Array.from({length: 54}, (_, i) =>
            `<option value="${i}" ${mapping.pin === i ? 'selected' : ''}>Pin ${i}</option>`
          ).join('')}
        </select>
      `;
      roomList.appendChild(div);
    });
  } else if (parsedData.response === "pinUpdated") {
    console.log(`Pin updated for room ${parsedData.roomId} to ${parsedData.pin}`);
  } else {
    //console.log("Received message:", data);
  }
  updateBodyHeatingClass();
}

let ws;
const wsStatusElement = document.getElementById("ws-status");

function setWsStatus(status) {
    if (!wsStatusElement) return;
    wsStatusElement.classList.remove("ws-connected", "ws-disconnected", "ws-error");
    switch (status) {
        case "connected":
            wsStatusElement.classList.add("ws-connected");
            wsStatusElement.title = "Połączono z WebSocket";
            break;
        case "disconnected":
            wsStatusElement.classList.add("ws-disconnected");
            wsStatusElement.title = "Rozłączono WebSocket, próba ponownego połączenia...";
            break;
        case "error":
            wsStatusElement.classList.add("ws-error");
            wsStatusElement.title = "Błąd połączenia WebSocket";
            break;
        default:
             wsStatusElement.classList.add("ws-disconnected");
             wsStatusElement.title = "Rozłączono WebSocket";
    }
}


function connectWebSocket() {
  //  ws = new WebSocket("ws://192.168.8.47:81");
  ws = new WebSocket("ws://netatmo_relay.local:81"); // Use mDNS name

  ws.onopen = function (event) {
    console.log("WebSocket connection opened:", event);
    setWsStatus("connected");
    // Send a message to the server if needed
    // ws.send(JSON.stringify({ message: "Hello Server!" }));
  };

  ws.onmessage = function (event) {
    console.log("Message from server:", event.data);
    handleWebSocketMessage(event.data);
  };

  ws.onclose = function (event) {
    console.log("WebSocket connection closed:", event);
    setWsStatus("disconnected");
    // Ponowne połączenie po 5 sekundach
    setTimeout(connectWebSocket, 5000); // Increased timeout to 5s
  };

  ws.onerror = function (error) {
    console.error("WebSocket error:", error);
    setWsStatus("error");
    // onclose will likely be called after onerror, attempting reconnect
  };
}

// Nawiązanie połączenia WebSocket przy całkowitym załadowaniu strony

document.addEventListener("DOMContentLoaded", function () {
  setWsStatus("disconnected"); // Set initial state
  connectWebSocket();
  console.log("DOM fully loaded and parsed");

  const configButton = document.querySelector(".configButton");
  const wifiConfigButton = document.querySelector("#wifiConfig");
  const modal = document.getElementById("configModal");
  const span = document.getElementsByClassName("close")[0];
  const saveButton = document.getElementById("saveConfig");
  const roomList = document.getElementById("roomList");
  const useGazButton = document.getElementById("useGazButton");
  const tabButtons = document.querySelectorAll(".tab-button");
  const tabContents = document.querySelectorAll(".tab-content");
  const saveMinimalTemp = document.querySelector("#configModal .manifoldMinTemp");


  // Obsługa zakładek
  tabButtons.forEach(button => {
    button.addEventListener("click", () => {
      // Usuń klasę active ze wszystkich przycisków
      tabButtons.forEach(btn => btn.classList.remove("active"));
      // Dodaj klasę active do klikniętego przycisku
      button.classList.add("active");

      // Ukryj wszystkie zakładki
      tabContents.forEach(content => content.style.display = "none");

      // Pokaż odpowiednią zakładkę
      const tabId = button.getAttribute("data-tab");
      document.getElementById(`${tabId}-tab`).style.display = "block";
    });
  });


  // on change input trigger function
  saveMinimalTemp.addEventListener("change", function() {
    ws.send(JSON.stringify({ command: "manifoldMinTemp", value: this.value }));
});


  wsSendCommand = (command, value) => {
    ws.send(JSON.stringify({ command: command, value: value }));
  };

  useGazButton.addEventListener("click", () => {
    if (ws.readyState === ws.OPEN) {
      if (useGazButton.classList.contains("active")) {
        wsSendCommand("usegaz", "false");
      } else {
        wsSendCommand("usegaz", "true");
      }
      useGazButton.classList.toggle("active");
    } else {
      alert("Brak połączenia z serwerem, odczekaj na połączenie");
    }
  });



  configButton.onclick = function () {
    modal.style.display = "block";
    if (ws.readyState === ws.OPEN) {
      ws.send(JSON.stringify({ command: "getRooms" }));
    } else {
      alert("Brak połączenia z serwerem, odczekaj na połączenie");
    }
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
    const roomItems = roomList.querySelectorAll(".room-item");

    roomItems.forEach((item) => {
      const nameInput = item.querySelector(".room-name");
      const pinSelect = item.querySelector(".room-pin");

      roomConfigs.push({
        id: nameInput.getAttribute("data-room-id"),
        name: nameInput.value,
        pinNumber: pinSelect.value,
      });
    });

    ws.send(JSON.stringify({
      command: "updateRooms",
      rooms: roomConfigs
    }));

    // Pokaż komunikat o zapisaniu
    const saveMessage = document.createElement("div");
    saveMessage.className = "save-message";
    saveMessage.textContent = "Konfiguracja zapisana";
    saveMessage.style.position = "absolute";
    saveMessage.style.top = "50%";
    saveMessage.style.left = "50%";
    saveMessage.style.transform = "translate(-50%, -50%)";
    saveMessage.style.backgroundColor = "#4CAF50";
    saveMessage.style.color = "white";
    saveMessage.style.padding = "15px";
    saveMessage.style.borderRadius = "5px";
    saveMessage.style.zIndex = "1000";

    modal.appendChild(saveMessage);

    setTimeout(() => {
      saveMessage.remove();
      modal.style.display = "none";
    }, 1500);
  };
});

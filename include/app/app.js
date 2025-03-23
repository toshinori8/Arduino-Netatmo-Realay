function showAlert(message, type = 'info') {
  const alertDiv = document.createElement('div');
  alertDiv.className = `p-4 mb-4 rounded-lg ${type === 'error' ? 'bg-red-500' : 'bg-blue-500'} text-white`;
  alertDiv.style.position = 'fixed';
  alertDiv.style.top = '20px';
  alertDiv.style.right = '20px';
  alertDiv.style.zIndex = '9999';
  alertDiv.textContent = message;
  
  document.body.appendChild(alertDiv);
  
  // Usuń alert po 5 sekundach
  setTimeout(() => {
    alertDiv.remove();
  }, 5000);
}

let dataIncoming = false;
let usegaz = document.querySelector(".usegaz");

const messageElements = document.querySelector(".console");
// WSaddress = "ws://192.168.8.33:8080";
WSaddress='ws://' + window.location.hostname + ':8080';
// WSaddress='ws://netatmo_relay.local/:8080';



const socket = new WebSocket(WSaddress);

const checkBoxes = function () {
  const checkboxes = document.querySelectorAll(
    '.checkbox-container input[type="checkbox"]'
  );

  for (let i = 0; i < checkboxes.length; i++) {
    checkboxes[i].addEventListener("click", function () {
      let state = this.checked;

      checkboxes[i].parentElement.querySelector(".handIcon").classList.toggle("disabled");

      let pin_number = checkboxes[i].getAttribute("ID")

      // create a JSON message with the pin number and state
      const message = {
        pin: pin_number,
        state: state ? "ON" : "SRODD",
        forced: state ? "true" : "DRALCE"
      };

      console.log(message);
      // send the message through the WebSocket

      socket.send(JSON.stringify(message));
    });

  }
};


///  USE GAZ BUTTON
usegaz.addEventListener("click", function (e) {
  e.preventDefault();
  this.classList.toggle("active");
  const message = {
    usegaz: this.classList.contains("active") ? "true" : "false"
  };
  socket.send(JSON.stringify(message));
})


window.addEventListener("load", function () {
  checkBoxes();
});

// nasłuchuj zdarzenia "close" na sockecie
// socket.addEventListener("close", () => {
//   // console.log('Połączenie websocket zostało zamknięte, trwa ponowne łączenie...');

//   messageElements.textContent = "WS Closed, Reconnecting...";

//   // utwórz nowe połączenie
//   const newSocket = new WebSocket(WSaddress);
//   // przypisz nowe połączenie do zmiennej `socket`
//   socket = newSocket;
// });

window.addEventListener("focus", function () {
  if (socket.readyState === WebSocket.CLOSED) {

    let newSocket = new WebSocket(WSaddress);
    //   // przypisz nowe połączenie do zmiennej `socket`
    socket = newSocket;

    // połączenie jest zamknięte, należy ponowić połączenie
    // socket = new WebSocket(WSaddress);
  }
});




socket.onmessage = function (event) {
  console.log("Received message:", event.data);
  const data = JSON.parse(event.data);
  
  if (data.type === 'debug_logs') {
    displayDebugLogs(data.logs);
    return;
  }

  if (data.response === "connected") {
    messageElements.textContent = "WS Connected";
    dataIncoming = true;
  }

  if (dataIncoming) {

    console.log(data)
    for (const key in data) {
      // Pobranie elementu HTML o identyfikatorze równym nazwie właściwości (np. "pin_4")

      // if (key == "piec_pompa") {
      //   const iconHeat = document.querySelector(".iconHeat");

      //   if (data[key] == "ON") {
      //     iconHeat.classList.add("active");
      //   } else {
      //     iconHeat.classList.remove("active");
      //   }
      // }

      if (key.includes("pin_")) {

        let element = document.getElementById(key);
        // Jeśli element istnieje i jest elementem "input type="checkbox""
        if (element && element.type === "checkbox") {
          // Ustawienie atrybutu "checked" elementu za pomocą wartości właściwości (np. "ON" lub "OFF")
          element.checked = data[key]["state"] === "ON";
        }
        const handIcon_ = document.getElementById(key).parentElement.querySelector(".handIcon");
        // Jeśli element istnieje i jest elementem "input type="checkbox""
        if (element && element.type === "checkbox") {
          // Ustawienie atrybutu "checked" elementu za pomocą wartości właściwości (np. "ON" lub "OFF")
          element.checked = data[key]["state"] === "ON";
          if (data[key]["forced"] === "true") {
            handIcon_.classList.remove("disabled");
          } else (handIcon_.classList.add("disabled"))
        }
      }





      const checkboxes = document.querySelectorAll(
        '.checkbox-container input[type="checkbox"]'
      );

      for (let i = 0; i < checkboxes.length; i++) {
        let checked = false;
        for (let j = 0; j < checkboxes.length; j++) {
          if (checkboxes[j].checked) {
            checked = true;
            break;
          }
        }

        document.body.classList.toggle("orange", checked);
      }
    }



  }



};

socket.onopen = function () {

ip=socket['url'].replace('ws:','https:').replace(':8080/','');



 


var ret = "data-123".replace('data-','');

  messageElements.textContent = "Connected";

  getForecast();
  generateQR(ip);

};

socket.onclose = function (e) {

  console.log('Socket is closed. Reconnect will be attempted in 15 second.', e.reason);
  let seconds = 15;

  function incrementSeconds() {
    seconds -= 1;
    console.log('Seconds :' + seconds)
    messageElements.textContent = "reconect: " + seconds;
  }

  var cancel = setInterval(incrementSeconds, 1000);

  setTimeout(function () {
    connect();
  }, 15000);
};

socket.onmessage = function(e) {
  console.log('Server: ', e.data);
  var data = JSON.parse(e.data);
  
  // Obsługa informacji o restarcie
  if (data.type === 'reset_info') {
    const resetReason = data.reason;
    const heap = data.heap;
    showAlert(`Urządzenie zostało zrestartowane. Powód: ${resetReason}, Pamięć: ${heap} bajtów`);
  }
  
  // Istniejąca obsługa innych wiadomości
  if (data.response === "connected") {
    messageElements.textContent = "WS Connected";
    dataIncoming = true;
  }

  if (dataIncoming) {

    console.log(data)
    for (const key in data) {
      // Pobranie elementu HTML o identyfikatorze równym nazwie właściwości (np. "pin_4")

      // if (key == "piec_pompa") {
      //   const iconHeat = document.querySelector(".iconHeat");

      //   if (data[key] == "ON") {
      //     iconHeat.classList.add("active");
      //   } else {
      //     iconHeat.classList.remove("active");
      //   }
      // }

      if (key.includes("pin_")) {

        let element = document.getElementById(key);
        // Jeśli element istnieje i jest elementem "input type="checkbox""
        if (element && element.type === "checkbox") {
          // Ustawienie atrybutu "checked" elementu za pomocą wartości właściwości (np. "ON" lub "OFF")
          element.checked = data[key]["state"] === "ON";
        }
        const handIcon_ = document.getElementById(key).parentElement.querySelector(".handIcon");
        // Jeśli element istnieje i jest elementem "input type="checkbox""
        if (element && element.type === "checkbox") {
          // Ustawienie atrybutu "checked" elementu za pomocą wartości właściwości (np. "ON" lub "OFF")
          element.checked = data[key]["state"] === "ON";
          if (data[key]["forced"] === "true") {
            handIcon_.classList.remove("disabled");
          } else (handIcon_.classList.add("disabled"))
        }
      }





      const checkboxes = document.querySelectorAll(
        '.checkbox-container input[type="checkbox"]'
      );

      for (let i = 0; i < checkboxes.length; i++) {
        let checked = false;
        for (let j = 0; j < checkboxes.length; j++) {
          if (checkboxes[j].checked) {
            checked = true;
            break;
          }
        }

        document.body.classList.toggle("orange", checked);
      }
    }



  }



};

// Dodaj na początku pliku po innych stałych
let debugLogs = [];
let isDebugVisible = false;

// Funkcja do wyświetlania logów debugowania
function displayDebugLogs(logs) {
  const debugLogsContainer = document.getElementById('debug-logs');
  if (!debugLogsContainer) return;

  // Podziel logi na linie i dodaj każdą jako osobny element
  const logLines = logs.split('\n');
  debugLogsContainer.innerHTML = ''; // Wyczyść poprzednie logi
  
  logLines.forEach(line => {
    if (line.trim()) { // Pomijamy puste linie
      const logLine = document.createElement('div');
      logLine.className = 'debug-log-line';
      logLine.textContent = line;
      debugLogsContainer.appendChild(logLine);
    }
  });

  // Przewiń do najnowszych logów
  debugLogsContainer.scrollTop = debugLogsContainer.scrollHeight;
}

// Obsługa przycisku do wyświetlania logów
document.addEventListener('DOMContentLoaded', function() {
  const debugToggle = document.getElementById('debug-toggle');
  const debugLogs = document.getElementById('debug-logs');
  let isDebugVisible = false;

  if (debugToggle && debugLogs) {
    debugToggle.addEventListener('click', function() {
      isDebugVisible = !isDebugVisible;
      debugLogs.style.display = isDebugVisible ? 'block' : 'none';
      // debugToggle.textContent = isDebugVisible ? 'Ukryj logi' : 'Pokaż logi';
    });
  }
});



// Stałe i zmienne globalne
const WSaddress = 'ws://netatmo_relay.local:8080';
const socket = new WebSocket(WSaddress);
let dataIncoming = false;

// Elementy DOM
const messageElements = document.querySelector(".console");
const usegaz = document.querySelector(".usegaz");

// Funkcje pomocnicze
function showAlert(message, type = 'info') {
  const alertDiv = document.createElement('div');
  alertDiv.className = `p-4 mb-4 rounded-lg ${type === 'error' ? 'bg-red-500' : 'bg-blue-500'} text-white`;
  alertDiv.style.position = 'fixed';
  alertDiv.style.top = '20px';
  alertDiv.style.right = '20px';
  alertDiv.style.zIndex = '9999';
  alertDiv.textContent = message;
  
  document.body.appendChild(alertDiv);
  
  setTimeout(() => {
    alertDiv.remove();
  }, 5000);
}

// Funkcja do aktualizacji stanu przekaźników
function updateRelayStates(relays) {

  
  if (relays && Array.isArray(relays)) {
    relays.forEach((relay, index) => {

      console.log(index);
      const element = document.getElementById(`pin_${index}`);
      if (element && element.type === "checkbox") {
        // Aktualizuj stan checkboxa
        element.checked = relay.isOn;
        
        // Aktualizuj ikonę ręki
        const handIcon = element.parentElement.querySelector(".handIcon");
        if (handIcon) {
          handIcon.classList.remove("disabled");
          handIcon.classList.toggle("enabled", relay.isOn);
        }
      
      }
    });

    // Sprawdź czy którykolwiek przekaźnik jest włączony
    const anyRelayOn = relays.some(relay => relay.isOn);
    document.body.classList.toggle("orange", anyRelayOn);
  }
}

// Funkcja do wyświetlania logów debugowania
function displayDebugLogs(logs) {
  const debugLogsContainer = document.getElementById('debugLog');
  if (!debugLogsContainer) return;

  const logLines = logs.split('\n');
  debugLogsContainer.innerHTML = '';
  
  logLines.forEach(line => {
    if (line.trim()) {
      const logLine = document.createElement('div');
      logLine.className = 'debug-log-line';
      logLine.textContent = line;
      debugLogsContainer.appendChild(logLine);
    }
  });

  debugLogsContainer.scrollTop = debugLogsContainer.scrollHeight;
}

// Obsługa checkboxów
function checkBoxes() {
  const checkboxes = document.querySelectorAll(
    '.checkbox-container input[type="checkbox"]'
  );

  checkboxes.forEach(checkbox => {
    checkbox.addEventListener("click", function () {
      let state = this.checked;
      let relayId = parseInt(this.getAttribute("ID").replace('pin_', ''));

      // Aktualizuj ikonę ręki
      const handIcon = this.parentElement.querySelector(".handIcon");
      handIcon.classList.remove("disabled");
      handIcon.classList.toggle("enabled", state);

      // Twórz wiadomość JSON w nowym formacie
      const message = {
        type: 'command',
        action: 'toggleRelay',
        relayId: relayId
      };

      // console.log("Wysyłanie wiadomości:", message);
      socket.send(JSON.stringify(message));
    });
  });
}

// Obsługa WebSocket
socket.onopen = function () {
  // console.log('WebSocket Connected');
  if (messageElements) {
    messageElements.textContent = "Connected";
  }
  dataIncoming = true;
};

socket.onclose = function (e) {
  // console.log('Socket is closed. Reconnect will be attempted in 15 second.', e.reason);
  if (messageElements) {
    messageElements.textContent = "Disconnected";
  }
  
  let seconds = 15;
  function incrementSeconds() {
    seconds -= 1;
    if (messageElements) {
      messageElements.textContent = "reconnect: " + seconds;
    }
  }

  var cancel = setInterval(incrementSeconds, 1000);
  setTimeout(function () {
    clearInterval(cancel);
    window.location.reload();
  }, 15000);
};

socket.onmessage = function (event) {
  // console.log("Otrzymano wiadomość:", event.data);
  const data = JSON.parse(event.data);
  
  if (data.type === 'debug_logs') {
    displayDebugLogs(data.logs);
    return;
  }

  if (data.type === 'state') {
    // console.log('Otrzymano stan:', data);
    
    // Aktualizuj temperaturę
    const manifoldTempElement = document.getElementById('manifoldTemp');
    if (manifoldTempElement) {
      manifoldTempElement.textContent = data.manifoldTemp.toFixed(1) + '°C';
    }
    
    // Aktualizuj stan pieca
    const woodStoveElement = document.getElementById('woodStove');
    if (woodStoveElement) {
      woodStoveElement.textContent = data.woodStove ? 'ON' : 'OFF';
    }
    
    // Aktualizuj stany przekaźników
    updateRelayStates(data.relays);
  }

  if (data.type === 'reset_info') {
    const resetReason = data.reason;
    const heap = data.heap;
    showAlert(`Urządzenie zostało zrestartowane. Powód: ${resetReason}, Pamięć: ${heap} bajtów`);
  }
};

// USE GAZ BUTTON
if (usegaz) {
  usegaz.addEventListener("click", function (e) {
    e.preventDefault();
    this.classList.toggle("active");
    const message = {
      usegaz: this.classList.contains("active") ? "true" : "false"
    };
    socket.send(JSON.stringify(message));
  });
}

// Inicjalizacja po załadowaniu strony
document.addEventListener('DOMContentLoaded', function() {
  const debugToggle = document.getElementById('debug-toggle');
  const debugLogs = document.getElementById('debugLog');
  let isDebugVisible = false;

  if (debugToggle && debugLogs) {
    debugToggle.addEventListener('click', function() {
      isDebugVisible = !isDebugVisible;
      debugLogs.style.display = isDebugVisible ? 'block' : 'none';
      this.textContent = isDebugVisible ? 'Ukryj logi' : 'Pokaż logi';
    });
  }

  // Inicjalizacja checkboxów
  checkBoxes();
});



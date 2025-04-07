
let dataIncoming = false;
let usegaz = document.querySelector(".usegaz");

const messageElements = document.querySelector(".console");
WSaddress = "ws://192.168.8.33:8080";
// WSaddress='ws://' + window.location.hostname + ':8080';
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
  const data = JSON.parse(event.data);


  if (data.response === "connected") {
    messageElements.textContent = "WS Connected";
    dataIncoming = true;
  }

  if (dataIncoming) {

    console.log(data)
    for (const key in data) {
      // Pobranie elementu HTML o identyfikatorze równym nazwie właściwości (np. "pin_4")

      if (key == "gaz") {
        const iconHeat = document.querySelector(".iconHeat");

        if (data[key] == "ON") {
          iconHeat.classList.add("active");
        } else {
          iconHeat.classList.remove("active");
        }
      }

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



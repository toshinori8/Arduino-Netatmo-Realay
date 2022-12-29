#include <Arduino.h>

const char webpage[] PROGMEM = R"rawliteral(

<head>
    <meta charset="utf-8" />
    <meta
      name="viewport"
      content="width=device-width,initial-scale=1,maximum-scale=1,minimum-scale=1"
    />
    <meta name="theme-color" content="#474E65" />
    <meta name="mobile-web-app-capable" content="yes" />
    <meta name="application-name" content="Netatmo Relay" />
    <title>Netatmo Relay</title>
    <link
      href="https://unpkg.com/tailwindcss@^2/dist/tailwind.min.css"
      rel="stylesheet"
    />
    <link
      href="https://fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700"
      rel="stylesheet"
      type="text/css"
    />
    <script
      type="text/javascript"
      src="https://unpkg.co/gsap@3/dist/gsap.min.js"
    ></script>
    <script
      type="text/javascript"
      src="https://unpkg.com/gsap@3/dist/Draggable.min.js"
    ></script>
    <script
      src="https://code.jquery.com/jquery-3.6.0.min.js"
      integrity="sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4="
      crossorigin="anonymous"
    ></script>
  
    <script
      type="module"
      src="https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.esm.js"
    ></script>
    <script
      nomodule
      src="https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.js"
    ></script>
  
    <style>
      * {
        border: 0;
        box-sizing: border-box;
        margin: 0;
        padding: 0;
      }
  
      :root {
        --l1: hsl(223, 10%%, 100%%);
        --l2: hsl(223, 10%%, 95%%);
        --l3: hsl(223, 10%%, 90%%);
        --l4: hsl(223, 10%%, 85%%);
        --l5: hsl(223, 10%%, 80%%);
        --l6: hsl(223, 10%%, 75%%);
        --text: hsl(223, 10%%, 20%%);
        font-size: calc(16px + (24 - 16) * (100vw - 320px) / (1280 - 320));
      }
  
      /* Dark theme */
      @media (prefers-color-scheme: dark) {
        :root {
          --l1: hsl(223, 10%%, 50%%);
          --l2: hsl(223, 10%%, 45%%);
          --l3: hsl(223, 10%%, 40%%);
          --l4: hsl(223, 10%%, 35%%);
          --l5: hsl(223, 10%%, 30%%);
          --l6: hsl(223, 10%%, 25%%);
          --green: hsla(78, 100%%, 27%%, 1);
          --text: hsl(223, 10%%, 80%%);
        }
      }
  
      @media (prefers-color-scheme: light) {
        :root {
          --green: hsla(78, 100%%, 27%%, 1);
        }
      }
  
      * {
        box-sizing: border-box;
      }
  
      body.orange {
        transition: all ease 0.5s;
        background-image: -webkit-linear-gradient(
          #fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00
        );
        background-image: -moz-linear-gradient(
          #fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00
        );
        background-image: -o-linear-gradient(
          #fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00
        );
        background-image: linear-gradient(
          #fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00
        );
  
        background-position: 100px;
      }
  
      body.connected {
        border: 1rem solid rgba(0, 0, 0, 0.1);
      }
  
      body {
        /*   background-image: transparent;*/
        border: 0rem solid rgba(0, 0, 0, 0);
        background-image: linear-gradient(
          rgba(5, 5, 5, 0.2),
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2)
        );
  
        transition: all ease 0.5s;
  
        /*	background: #34495e;*/
        font-family: 'Open Sans';
        display: flex;
        align-items: center;
        justify-content: center;
        height: 100vh;
        margin: 0;
      }
  
      .container {
        background: rgba(200, 200, 200, 1);
        display: flex;
        align-items: center;
        justify-content: center;
        flex-direction: column;
        overflow: hidden;
        text-align: center;
        width: 280px;
        height: 170px;
      }
  
      .container h3 {
        color: #111;
        margin: 0 0 25px;
        position: relative;
        z-index: 2;
      }
  
      .checkbox-container {
        display: inline-block;
        position: relative;
      }
  
      .checkbox-container label {
        background-color: #aaa;
        border: 1px solid #fff;
        border-radius: 20px;
        display: inline-block;
        position: relative;
        transition: all 0.3s ease-out;
        width: 45px;
        height: 25px;
        z-index: 2;
      }
  
      .checkbox-container label::after {
        content: ' ';
        background-color: #fff;
        border-radius: 50%;
        position: absolute;
        top: 1.5px;
        left: 1px;
        transform: translateX(0);
        transition: all 0.5s ease;
        width: 20px;
        height: 20px;
        z-index: 3;
      }
  
      .checkbox-container input {
        visibility: hidden;
        position: absolute;
        z-index: 2;
      }
  
      .checkbox-container input:checked + label + .active-circle {
        transform: translate(-50%, -50%) scale(15);
      }
  
      .checkbox-container input:checked + label::after {
        transition: transform 0.5s ease-out;
        transform: translateX(calc(100% + 0.5px));
      }
  
      .checkbox-container input:checked + label::after {
        background-color: indianred;
      }
  
      .active-circle {
        border-radius: 50%;
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(calc(-2000% - 0px), calc(-10% - 2px)) scale(20);
        transition: transform 0.5s ease-out;
        width: 30px;
        height: 30px;
        z-index: 1;
      }
  
      .checkbox-container.purple .active-circle,
      .checkbox-container.purple input:checked + label {
        background: linear-gradient(
            to bottom,
            rgba(255, 255, 255, 0.15) 0%,
            rgba(0, 0, 0, 0.15) 100%
          ),
          radial-gradient(
              at top center,
              rgba(255, 255, 255, 0.4) 0%,
              rgba(0, 0, 0, 0.4) 120%
            )
            #989898;
        background-blend-mode: multiply, multiply;
      }
  
      .checkbox-container.purple label {
        border-color: rgba(0, 0, 0, 0.1);
        background: rgba(0, 0, 0, 0.2) !important;
      }
  
      @media (max-width: 840px) {
        body {
          flex-direction: column;
        }
      }
  
      footer {
        background-color: #222;
        color: #fff;
        font-size: 14px;
        bottom: 0;
        position: fixed;
        left: 0;
        right: 0;
        text-align: center;
        z-index: 999;
      }
  
      footer p {
        margin: 10px 0;
      }
  
      footer i {
        color: red;
      }
  
      footer a {
        color: #3c97bf;
        text-decoration: none;
      }
  
      body {
        margin-top: 120px;
      }
  
      .containerouter {
        position: absolute;
        /*        overflow: hidden;*/
        padding-bottom: 200px;
        padding-top: 110px;
        border-radius: 20px;
        box-shadow: 0px 0px 122px rgba(10, 10, 10, 0.2);
      }
  
      footer .console {
        z-index: 200;
        position: absolute;
        right: 20px;
        top: 8px;
      }
        footer .iconHeat{
            width: 30px;
            height: 30px;
            position: absolute;
            right: 120px;
            top: 15px;
            opacity:0.3;
              transition: opacity 0.3s ease;

        }
        footer .iconHeat.active {
                opacity: 1;
        }
      footer ion-icon {
        padding: 11px;
  
        color: white !important;
        align-self: flex-start;
        display: flex;
        transition: all ease 0.5s;
      }
  
      footer ion-icon:hover {
        transform: rotate(20deg);
      }
    </style>
  </head>
  
  <html>
    <body class="">
      <!--  <div class="container">  -->
  
      <br /><br /><br /><br /><br /><br /><br /><br />
      <div class="containerouter">
        <div class="container">
          <h3>AE Sypialnia</h3>
          <div class="checkbox-container purple">
            <input type="checkbox" id="pin_0" />
            <label for="pin_0"></label>
            <div class="active-circle"></div>
          </div>
        </div>
  
        <div class="container">
          <h3>Waleria</h3>
          <div class="checkbox-container purple">
            <input type="checkbox" id="pin_1" />
            <label for="pin_1"></label>
            <div class="active-circle"></div>
          </div>
        </div>
  
        <div class="container">
          <h3>Kuchnia</h3>
          <div class="checkbox-container purple">
            <input type="checkbox" id="pin_2" />
            <label for="pin_2"></label>
            <div class="active-circle"></div>
          </div>
        </div>
  
        <div class="container">
          <h3>Łazienka</h3>
          <div class="checkbox-container purple">
            <input type="checkbox" id="pin_3" />
            <label for="pin_3"></label>
            <div class="active-circle"></div>
          </div>
        </div>
  
        <div class="container">
          <h3>Przedpokoj</h3>
          <div class="checkbox-container purple">
            <input type="checkbox" id="pin_4" />
            <label for="pin_4"></label>
            <div class="active-circle"></div>
          </div>
        </div>
  
        <div class="container">
          <h3>Klatka</h3>
          <div class="checkbox-container purple">
            <input type="checkbox" id="pin_5" />
            <label for="pin_5"></label>
            <div class="active-circle"></div>
          </div>
        </div>
      </div>
      <!--    </div>-->
      <footer>
        <a href="config">
          <ion-icon name="cog-outline" size="large"></ion-icon>
        </a>
  
        <p>
<svg class="iconHeat" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" width="980px"
	 height="778.2px" viewBox="0 0 980 778.2" style="overflow:visible;enable-background:new 0 0 980 778.2;" xml:space="preserve">
<style type="text/css">
	.st0{fill:#FD8300;}
</style>
<defs>
</defs>
<g>
	<g transform="translate(0.000000,511.000000) scale(0.100000,-0.100000)">
		<path class="st0" d="M8845.3,4724.7c480.4-579.6,456.6-492.3,206.5-766.2c-369.2-397-500.2-913.1-369.2-1429.2
			c75.4-301.7,246.2-678.9,412.9-921.1c345.3-504.2,619.3-1238.7,686.8-1854.1c71.5-623.3-75.4-1250.6-401-1742.9
			c-166.8-246.2-611.4-698.8-670.9-674.9c-39.7,11.9-718.6,889.3-718.6,929c0,11.9,43.7,55.6,91.3,99.3
			c170.7,131,381.1,484.4,448.6,750.4c131,516.1-15.9,1079.9-484.4,1869.9c-536,905.2-730.5,1679.4-603.4,2386
			c75.4,412.9,202.5,726.5,408.9,1044.1c166.8,250.1,571.7,694.8,635.2,694.8C8511.9,5109.8,8670.7,4935.1,8845.3,4724.7z"/>
		<path class="st0" d="M5184.9,4665.1l377.2-444.6l-198.5-222.3c-285.8-317.6-397-579.6-420.8-952.8
			c-27.8-504.2,87.3-861.5,492.3-1516.6c258-420.9,412.8-762.3,539.9-1206.9c71.5-254.1,91.3-412.9,91.3-849.6
			c0-619.3-71.5-913.1-325.6-1349.8c-150.9-254.1-639.2-794-718.6-794c-23.8,0-206.4,210.4-401,472.5l-357.3,468.5l178.7,174.7
			c99.3,95.3,226.3,274,285.9,393.1c99.2,198.5,107.2,258.1,103.2,674.9c0,555.8-71.5,770.2-492.3,1468.9
			c-488.3,821.8-663,1409.4-623.3,2124C3751.7,3780,3989.9,4308,4490.2,4840c138.9,146.9,270,270,289.8,270
			C4795.9,5109.8,4982.4,4907.3,5184.9,4665.1z"/>
		<path class="st0" d="M1441.1,4704.8c186.6-222.3,353.3-428.8,369.2-456.6c23.8-39.7-19.9-115.1-139-242.2
			c-238.2-258.1-337.5-428.8-412.9-726.5c-123.1-484.3,7.9-1000.5,440.7-1707.1c484.4-794,667-1365.7,671-2100.2
			c0-532-67.5-833.7-285.9-1250.6c-142.9-274-682.9-893.3-778.2-893.3c-35.7,0-595.5,702.7-706.7,893.3
			c-15.9,27.8,43.7,123,154.8,238.2c337.5,353.4,464.5,813.9,373.2,1349.8c-63.5,365.3-186.6,655.1-512.1,1202.9
			C158.8,1782.9,0,2283.1,0,2946.1c0,591.5,146.9,1056,496.3,1556.3c150.9,222.3,516.1,607.4,571.7,607.4
			C1087.8,5109.8,1258.5,4927.2,1441.1,4704.8z"/>
	</g>
</g>
</svg>
</p>  
          
        <p class="console"></p>
      </footer>
  
      <!-- to remove the hover effect just remove the 2 following classes from the code above 'group' (inside label) and 'group-hover:after:translate-x-1' inside span -->
  
      <script type="text/javascript">
        const checkBoxes = function () {
          const checkboxes = document.querySelectorAll(
            '.checkbox-container input[type="checkbox"]'
          );
  
          for (let i = 0; i < checkboxes.length; i++) {
            checkboxes[i].addEventListener('change', function () {
              let checked = false;
              for (let j = 0; j < checkboxes.length; j++) {
                if (checkboxes[j].checked) {
                  checked = true;
                  break;
                }
              }
              document.body.classList.toggle('orange', checked);
            });
          }
        };
  
        window.addEventListener('load', function () {
          checkBoxes();
        });
  
        const socket = new WebSocket(
          'ws://' + window.location.hostname + ':8080'
        );
//          const socket = new WebSocket(
//          'ws://192.168.8.70:8080'
//        );
  
          
        
  
        socket.onmessage = function (event) {
          const data = JSON.parse(event.data);
          //
            
          if (data.response === 'connected') {
            const messageElements = document.getElementsByClassName('console');
            messageElements[0].innerText = 'Connected';
          }
       
  
          for (const key in data) {
            // Pobranie elementu HTML o identyfikatorze równym nazwie właściwości (np. "pin_4")
              
              if(key=="piec_pompa"){
                   const iconHeat = document.querySelector('.iconHeat');
                       
                  
                  if(data[key]=="ON"){
                                        iconHeat.classList.add('active');

                      
                  } else {
    iconHeat.classList.remove('active');
  }

                  
              }
            const element = document.getElementById(key);
            // Jeśli element istnieje i jest elementem "input type="checkbox""
            if (element && element.type === 'checkbox') {
              // Ustawienie atrybutu "checked" elementu za pomocą wartości właściwości (np. "ON" lub "OFF")
              element.checked = data[key] === 'ON';
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
            const message = {
            pin: i,
              state: checked
              };

              console.log(message);

              // send the message through the WebSocket
              webSocket.send(JSON.stringify(message));

              document.body.classList.toggle('orange', checked);
            }
          }
        };
          
        const xhr = new XMLHttpRequest();
const consoleElement = document.querySelector('.console');

function checkServerAvailability() {
  xhr.open('GET', window.location.hostname);
  xhr.onload = () => {
    if (xhr.status === 200) {
      consoleElement.textContent = 'Connected';
    } else {
      consoleElement.textContent = 'Disconected';
    }
  };
  xhr.send();
}

setInterval(checkServerAvailability, 300000); // co 5 minut (300000 milisekund)  
          
          
      </script>
    </body>
  </html>


)rawliteral";

#include <Arduino.h>

const char webpage[] PROGMEM = R"rawliteral(


<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,minimum-scale=1">
    <meta name="theme-color" content="#474E65">
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="application-name" content="Netatmo Relay">
    <title>Netatmo Relay</title>
    
<link href="https://unpkg.com/tailwindcss@^2/dist/tailwind.min.css" rel="stylesheet">
    <link href='https://fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700' rel='stylesheet' type='text/css'>
    <script type="text/javascript" src="https://unpkg.co/gsap@3/dist/gsap.min.js"></script>
    <script type="text/javascript" src="https://unpkg.com/gsap@3/dist/Draggable.min.js"></script>
    <script src="https://code.jquery.com/jquery-3.6.0.min.js" integrity="sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4=" crossorigin="anonymous"></script>

<script type="module" src="https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.esm.js"></script>
<script nomodule src="https://unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.js"></script>

   

        

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

    body.orange{
        transition: all ease 0.5s;
            background-image: -webkit-linear-gradient(#fd8000,#fe8a00 20%,#fe9300 40%,#ff9c00 60%,orange 80%,#ffae00);
    background-image: -moz-linear-gradient(#fd8000,#fe8a00 20%,#fe9300 40%,#ff9c00 60%,orange 80%,#ffae00);
    background-image: -o-linear-gradient(#fd8000,#fe8a00 20%,#fe9300 40%,#ff9c00 60%,orange 80%,#ffae00);
    background-image: linear-gradient(#fd8000,#fe8a00 20%,#fe9300 40%,#ff9c00 60%,orange 80%,#ffae00);
      
        
        
         background-position:100px;
    }
    
    
  
    
    body.connected{
            
          border: 1rem solid rgba(0,0,0,0.1);
    } 
body {
   
/*   background-image: transparent;*/
   border: 0rem solid rgba(0,0,0,0);
    background-image: linear-gradient(rgba(5,5,5,0.2),rgba(5,5,5,0.2) 0%,rgba(5,5,5,0.2) 0%,rgba(5,5,5,0.2) 0%,rgba(5,5,5,0.2) 0%,rgba(5,5,5,0.2));
  
    
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
	background: rgba(200,200,200,1);
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
    .checkbox-container input:checked + label::after{
        
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
	background: linear-gradient(to bottom, rgba(255,255,255,0.15) 0%, rgba(0,0,0,0.15) 100%), radial-gradient(at top center, rgba(255,255,255,0.40) 0%, rgba(0,0,0,0.40) 120%) #989898;
 background-blend-mode: multiply,multiply;
}

    .checkbox-container.purple label{
        
       border-color: rgba(0,0,0,0.1);
    background: rgba(0,0,0,0.2) !important;
            
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
    body{
        
        margin-top: 120px;
    }
   .containerouter
    {    
        position: absolute;
/*        overflow: hidden;*/
        padding-bottom: 200px;
        padding-top: 110px;
        border-radius: 20px;
            box-shadow: 0px 0px 122px rgba(10,10,10,0.2);
    
    }
         footer .console{
                
                z-index: 200;
    position: absolute;
    right: 20px;
    top: 8px;
                
            }
        footer ion-icon {
             padding: 11px;

    color: white !important;
    align-self: flex-start;
    display: flex;
            transition: all ease 0.5s;
        }
            footer ion-icon:hover{
                
                transform: rotate(20deg);
            }
    </style>

</head>

<body class="">
<!--  <div class="container">  -->

<br><br><br><br><br><br><br><br>
<div class="containerouter">   
    
  <div class="container">
	<h3>AE Sypialnia</h3>
	<div class="checkbox-container purple">
		<input type="checkbox"  id="pin_0" />
		<label for="pin_0"></label>
		<div class="active-circle"></div>
	</div>
</div>

<div class="container">
	<h3>Waleria</h3>
	<div class="checkbox-container purple">
		<input type="checkbox"  id="pin_1" />
		<label for="pin_1"></label>
		<div class="active-circle"></div>
	</div>
</div>

<div class="container">
	<h3>Kuchnia</h3>
	<div class="checkbox-container purple">
		<input type="checkbox"  id="pin_2" />
		<label for="pin_2"></label>
		<div class="active-circle"></div>
	</div>
</div>
    
    
  <div class="container">
	<h3>Łazienka</h3>
	<div class="checkbox-container purple">
		<input type="checkbox"  id="pin_3" />
		<label for="pin_3"></label>
		<div class="active-circle"></div>
	</div>
</div>
    
      <div class="container">
	<h3>Przedpokoj</h3>
	<div class="checkbox-container purple">
		<input type="checkbox"  id="pin_4" />
		<label for="pin_4"></label>
		<div class="active-circle"></div>
	</div>
</div>
    
      <div class="container">
	<h3>Klatka</h3>
	<div class="checkbox-container purple">
		<input type="checkbox"  id="pin_5" />
		<label for="pin_5"></label>
		<div class="active-circle"></div>
	</div>
</div>
    
  
</div>
<!--    </div>-->
<footer>
    <a href='config'>
    <ion-icon name="cog-outline" size="large"></ion-icon>
        
    </a>
    
    <p class="console"></p>
    
    
        
</footer>


<!-- to remove the hover effect just remove the 2 following classes from the code above 'group' (inside label) and 'group-hover:after:translate-x-1' inside span -->

        
      





    
    <script type="text/javascript">

const checkBoxes = function(){
    
const checkboxes = document.querySelectorAll('.checkbox-container input[type="checkbox"]');

for (let i = 0; i < checkboxes.length; i++) {
  checkboxes[i].addEventListener('change', function() {
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
}
    
        
        window.addEventListener('load', function() {
  checkBoxes();
});
        
        
    const socket = new WebSocket("ws://" + window.location.hostname+":8080");
    
//const socket = new WebSocket('ws://192.168.8.70:8080');

socket.onmessage = function(event) {
    const data = JSON.parse(event.data);
//
    if (data.response === 'connected') {
   
        console.log("Connected to MCU");
      const messageElements =  document.getElementsByClassName("console");
            messageElements[0].innerText="Connected";
    
    }
//   
    console.log(event);
    
//  Object.keys(data).forEach(key => {
//    const value = data[key];
//    const button = document.querySelector(`input[name="${key}"]`);
//
//    if (button) {
//      button.checked = value === 'on';
//    }
//  });
    
    for (const key in data) {
    // Pobranie elementu HTML o identyfikatorze równym nazwie właściwości (np. "pin_4")
    const element = document.getElementById(key);
    // Jeśli element istnieje i jest elementem "input type="checkbox""
    if (element && element.type === "checkbox") {
      // Ustawienie atrybutu "checked" elementu za pomocą wartości właściwości (np. "ON" lub "OFF")
      element.checked = (data[key] === "ON");
         
    }
        
    const checkboxes = document.querySelectorAll('.checkbox-container input[type="checkbox"]');

for (let i = 0; i < checkboxes.length; i++) {
  let checked = false;
    for (let j = 0; j < checkboxes.length; j++) {
      if (checkboxes[j].checked) {
        checked = true;
        break;
      }
    }
    document.body.classList.toggle('orange', checked);
}
        
        
  }
 
    
};





      








          

    </script>

</body>

</html>


)rawliteral";

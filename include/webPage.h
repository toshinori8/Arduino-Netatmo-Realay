#include <Arduino.h>

 const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
  <head>
    <meta name="theme-color" content="#474E65" />
    <meta name="mobile-web-app-capable" content="yes" />
    <meta name="application-name" content="Netatmo Relay" />

    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Netatmo Relay</title>
    <style type="text/css">* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
}

/* Dark theme */
@media (prefers-color-scheme: dark) {
    :root {
      --l1: hsl(223, 10%, 50%);
      --l2: hsl(223, 10%, 45%);
      --l3: hsl(223, 10%, 40%);
      --l4: hsl(223, 10%, 35%);
      --l5: hsl(223, 10%, 30%);
      --l6: hsl(223, 10%, 25%);
      --green: hsla(78, 100%, 27%, 1);
      --text: hsl(223, 10%, 80%);
    }
  }
  
  @media (prefers-color-scheme: light) {
    :root {
      --green: hsla(78, 100%%, 27%%, 1);
    }
  }


body {
   
    background-image: linear-gradient(rgba(5, 5, 5, 0.2), rgba(5, 5, 5, 0.2) 0%, rgba(5, 5, 5, 0.2) 0%, rgba(5, 5, 5, 0.2) 0%, rgba(5, 5, 5, 0.2) 0%, rgba(5, 5, 5, 0.2));
    transition: all ease 0.5s;
    min-height: 100vh;
    transition: background-color 0.5s ease;
}


/* no select class */

.no-select {
    -webkit-touch-callout: none; /* iOS Safari */
    -webkit-user-select: none; /* Safari */
    -khtml-user-select: none; /* Konqueror HTML */
    -moz-user-select: none; /* Old versions of Firefox */
    -ms-user-select: none; /* Internet Explorer/Edge */
    user-select: none; /* Non-prefixed version, currently supported by Chrome, Edge, Opera and Firefox */
  }

  /* no select for br, button, li, ul, body */
    body, button, ul, li, br, a {
    -webkit-touch-callout: none; /* iOS Safari */
    -webkit-user-select: none; /* Safari */
    -khtml-user-select: none; /* Konqueror HTML */
    -moz-user-select: none; /* Old versions of Firefox */
    -ms-user-select: none; /* Internet Explorer/Edge */
    user-select: none; /* Non-prefixed version, currently supported by Chrome, Edge, Opera and Firefox */
    }





@keyframes fadeIn {
    from {
      opacity: 0;
    }
    to {
      opacity: 1;
    }
  }


.container .imageBack_container {
    position: fixed;
    z-index: 0;
    right: -24%;
    bottom: 0px;
    width: 80%;
    overflow: hidden;
    opacity: 0.05;
    transform: scale(0.9);
    overflow: hidden;
}




body.heating::before {
    content: "";
    background: rgba(255, 174, 0, 1.000);
    position: absolute;
    top: 0px;
    right: 0px;
    bottom: 0px;
    left: 0px;
    opacity: 1;
    transition: all ease 1s;
    background-image: -webkit-linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-image: -moz-linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-image: -o-linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-image: linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-position: 100px;
}

body::before {
    content: "";
    background: rgba(255, 174, 0, 1.000);
    position: absolute;
    opacity: 0;
    top: 0px;
    right: 0px;
    bottom: 0px;
    left: 0px;
    transition: all ease 1s;
    background-image: -webkit-linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-image: -moz-linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-image: -o-linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-image: linear-gradient(#fd8000, #fe8a00 20%, #fe9300 40%, #ff9c00 60%, orange 80%, #ffae00);
    background-position: 100px;
}



.container {
    max-width: 480px;
    margin: 0 auto;
    padding: 20px;
    padding-bottom: 80px;
    min-height: 100vh;
    position: relative;
}

.thermostat-list {
    padding: 20px 0;
    display: flex;
    flex-direction: column;
    gap: 16px;
    max-height: calc(100vh - 140px);
    /* overflow-; */
    scroll-behavior: smooth;
}
.thermostat-card {
    margin-top: 20px;
    transition: transform all 0.5s ease;
    animation: fadeIn 1s;
}

.thermostat-card.expanded {
    /* width: 95vw; */
    height: 449px;
    /*    left: -20vh; */
}

/* .thermostat-card.expanded {
    position: fixed !important;
width: 100%;
z-index: 10000;
height: 100%;
top: 0;
left: 0;
animation: expandToFullScreen 0.5s forwards;

} */
@keyframes shrinkToNormal {
0% {
transform: scale(1);
}
100% {
transform: scale(0.4);
}
}
@keyframes expandToFullScreen {
0% {
transform: scale(0.4);
opacity: 0;
}
50% {
opacity: 1;
}
100% {
transform: scale(1);
opacity: 1;
}

}
.thermostat-card {
    border-top: 3px solid #66666687;
    background: white;
    padding: 29px;
    border-radius: 20px;
    /* border-bottom: 0.6px darkred solid; */
    1: 29px;
    box-shadow: 16px 26px 70px 30px rgba(0,0,0,0.1);
    cursor: pointer;
    transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    position: relative;
    transform-origin: center;
    height: 210px;!;!i;!;
    z-index: 1;
    overflow: hidden;
    /* animation: shrinkToNormal 0.5s backwards; */
}

.thermostat-list .battery_indicator{


}
.thermostat-card .battery_indicator{
  opacity: 0;
  position: relative;
  top: -200px;
  transform: scale(0.5) scaleY(0.7);
  left: 365px;
  width: 46px;
  transition: all 0.0s ease !important;!i;!;
}
.thermostat-card.expanded .battery_indicator{
  opacity: 1;
  position: relative;
  top: -329px;
  transform: scale(0.5) scaleY(0.7);
  left: 365px;
  width: 46px;
  transition: all ease 0.3s !important;!i;!;
}
.thermostat-card h2 {
    font-size: 20px;
    margin-bottom: 12px;
    color: #333;
    transition: all 0.4s ease;
    padding-right: 24px;
    max-width: 50%;
}

.expanded h2 {
    /* font-size: 24px; */
    margin-left: -15px;
    background: #c7c6c6;
    max-width: 50%;
    padding: 12px;
    margin-top: -14px;
    color: white;
    text-align: left;
    border-radius: 30px;
    box-shadow: inset 0px 0px 13px 4px rgba(0,0,0,0.1);
    font-weight: 900;
    padding-left: 43px;
}

.temperatures {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin: 20px 0;
    transition: all 2s ease;
}

.temperature {
    text-align: center;
}

.temperature-value {
    font-size: 32px;
    font-weight: 300;
    color: #1a1a1a;
}

.temperature-label {
    font-size: 12px;
    color: #666;
    margin-top: 4px;
    text-align: left;
}

.expanded .temperatures {
    flex-direction: row;
    gap: 20px;
    margin: 40px 0;
}

.expanded .heating-indicator {
  top: 341px;
  right: 24px;
}

.expanded .temperature-label {
    /* font-size: 14px; */
}

.controls {
    height: 0;
    opacity: 0;
    transform: translateY(-20px);
    transition: all 0.8s ease;
    overflow: hidden;
}

.expanded .controls {
    height: auto;
    opacity: 1;
    transform: translateY(0);
    padding-top: 20px;
}

.temperature-slider {
    width: 100%;
    margin: 20px 0;
    height: 20px;
    -webkit-appearance: none;
    background: #f0f0f0;
    border-radius: 10px;
    outline: none;
}

.temperature-slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 28px;
    height: 28px;
    background: #ff6b00;
    border-radius: 50%;
    cursor: pointer;
    box-shadow: 0 2px 6px rgba(255, 107, 0, 0.3);
}

.quick-actions {
    display: flex;
    gap: 8px;
    margin-top: 12px;
}

.action-button {
    padding: 8px 16px;
    border: 1px solid #e0e0e0;
    border-radius: 20px;
    background: transparent;
    color: #666;
    font-size: 12px;
    cursor: pointer;
    transition: all 0.2s ease;
}

.action-button:hover {
    background: #f5f5f5;
    border-color: #ccc;
}

.action-button.a_fire svg path{

  fill: #b9b9b9;
  stroke: #111111;
  stroke-width: 4px;


}
.action-button.a_fire svg{
width: 20px;

}
.action-button.active{

  background-color: #3c97bf;
}
.action-button.active.a_fire svg path{

  fill: #f6f5f5;
  stroke: #f8f8f8;
  stroke-width: 4px;


}

.footer a{
    color: white;
}

.footer .iconHeat {
    width: 30px;
    height: 30px;
    position: absolute;
    right: 120px;
    top: 15px;
    opacity: 0.3;
    transition: opacity 0.3s ease;
  }
  
    footer p.heatState {
   
    
    opacity: 0.3;
    transition: opacity 0.3s ease;
  }

  footer .heatState.active {
   

    opacity: 0.7;
    transition: opacity 0.3s ease;
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


footer {
    display: flex;
    background-color: #222;
    color: #fff;
    font-size: 14px;
    bottom: 0;
    position: fixed;
    left: 0;
    right: 0;
    text-align: center;
    z-index: 2000;
  }
  
footer {

    #ws-status svg .st1_wsConnect {
        fill: #ccc;
        opacity: 0.6;
    }
    #ws-status.connected svg .st1_wsConnect {
        fill: #FB8725;
        opacity: 1;
    }
    #ws-status svg .st0_wsConnect {
        fill: #FB8725;
        opacity: 0.5;
    }
    
    #ws-status.connected svg .st1_wsConnect {
        fill: #FB8725;
        opacity: 0.8;
    }
}

 
  
  footer i {
    color: red;
  }
  
  footer a {
    transition: all 0.3s ease-in;
    float: left;
    color: #3c97bf;
    position: relative;
    text-decoration: none;
    height: 52px;
    width: 54px;
  }
  
  footer a:hover {
  
    background-color: rgba(185, 66, 67, 1.000);
  
  }
  footer .right{

    position: fixed;
    right: 13px;

    p{

        margin-left: 12px;
    }

  }
  

  footer a:active {
    background-color: rgba(185, 66, 67, 1.000);
  
  }
  footer p{
    width: 30px;
    height: 30px;
    float: right !important;
    margin: 10px 0;
    svg {
      width: 30px;
      height: 30px;
    }   
}

footer a svg, footer p svg {

    transition: all ease 0.5s !important;
}
 
footer .a_fire svg {

    transform: scale(0.8);

}

footer a:hover svg, footer p:hover svg{

    transform: rotate(10deg);

}

footer .a_weather img{

width: 39px;
margin-top: 4px;

}

footer .a_qrcode svg{
transform: scale(0.8);

}
footer .a_qrcode svg rect{
fill: #fff;
stroke-linejoin: round;
    
}
footer .a_qrcode svg rect.ionicon-fill-none{
    stroke-width: 1;
    fill: none;
    stroke: #fff;
}

footer a svg {
    width: 35px;
    height: 35px;
    margin: 7px 2px;
   
   
  }
  footer .usegaz svg{
    width: 35px;
    height: 35px;
    margin: 7px 2px;
  }
  footer .usegaz svg rect{
    stroke: #fff;
    fill: none;
    outline: #f5f5f5;
    stroke-linejoin: round;
    stroke-width:  22px;

  }


footer a svg path { fill: rgba(250, 250, 250, 0.8);}

  footer a.usegaz {}
  
  footer a.usegaz.active {
  
    background-color: rgba(185, 66, 67, 1.000);
  
  }
  
  footer a.usegaz svg {
    width: 24px;
  
  }
  
  
  
  footer a:hover:after {
  
    position: absolute;
    background-color: red;
  
  }
  



/* Loader styles */
.loader {
    /* display: none; */
    justify-content: center;
    align-items: center;
    height: 100vh;
    position: absolute;
    left: 50%;
    top: 40%;
    margin-left: -50px;
    margin-top: -130px;
    width: 13%;
    height: 2%;
}

.loader svg {
    /* animation: rotate 2s linear infinite; */
}

.loader svg circle {
    /* fill: none; */
    /* stroke: #333; */
    /* stroke-width: 24; */
    /* stroke-dasharray: 383; */
    /* stroke-dashoffset: 430; */
    /* animation: dash 1.5s ease-in-out infinite; */
}


loader.loader {
    width: 65px;
    height: 30px;
    position: relative;
  }
  .loader:before {
    content: "";
    position: absolute;
    border-radius: 28px;
    box-shadow: 0 0 0 20px inset #2d2d2d;
    animation: l3 0.75s infinite alternate;
    fill: azure;
    /* color: gold; */
    opacity: 0.25;
  }
  @keyframes l3 {
    0% {
      inset: 0 35px 0 0;
    }
    50% {
      inset: 0 0 0 0;
    }
    100% {
      inset: 0 0 0 35px;
    }
  }




.status-bar {
    position: fixed;
    bottom: 0;
    left: 0;
    right: 0;
    height: 80px;
    background: rgba(20, 20, 20, 0.649);
    box-shadow: 0 -2px 10px rgba(0, 0, 0, 0.1);
    display: flex
;
    justify-content: flex-start;
    align-items: center;
    padding: 0 20px;
    z-index: 1000;
    flex-direction: row;
}

.status-item {
    width: 8%;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 4px;
    font-size: 12px;
    color: #666;
    cursor: pointer;
    margin-left: 12px;
}

.status-item svg {

    outline: #ccc;
}

.status-item span {   
     transition: all 0.2s ease;
}
    .status-item:hover span {
    color: white;

}

.status-item.active {
    color: #ff6b00;
}

.status-item .status-icon {
    font-size: 20px;
    margin-bottom: 4px;
}

/*  */
/* .expanded .heating-indicator { */
    /* top: 184px; */
    /* right: 24px; */
/* } */
.heating-indicator {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #ff6b00;
    position: absolute;
    top: 184px;
    right: 24px;
    animation: pulse 0.9s infinite;
}

@keyframes pulse {
    0% { transform: scale(1); opacity: 1; }
    50% { transform: scale(1.5); opacity: 0.5; }
    100% { transform: scale(1); opacity: 1; }
}

.back-button {
    position: absolute;
    top: 15px;
    left: 17px;
    width: 32px;
    height: 32px;
    border: none;
    background: none;
    cursor: pointer;
    opacity: 0;
    transform: translateX(-10px);
    transition: all 0.3s ease;
    display: flex;
    align-items: center;
    justify-content: center;
    color: #333;
    font-size: 24px;
}

.expanded .back-button {
    opacity: 1;
    transform: translateX(0);
    color: white;
    font-weight: 900;
}

.weather-panel {
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 80px;
    background: white;
    z-index: 200;
    padding: 24px;
    transform: translateY(100%);
    transition: transform 0.3s ease;
}

.weather-panel.visible {
    transform: translateY(0);
}

.weather-content {
    text-align: center;
    padding: 40px 20px;
}

.weather-temp {
    font-size: 72px;
    font-weight: 300;
    margin: 20px 0;
}

.weather-desc {
    font-size: 24px;
    color: #666;
    margin-bottom: 40px;
}

@keyframes expandToFullScreen {
    0% {
    transform: scale(0.4);
    opacity: 0;
    }
    50% {
    opacity: 1;
    }
    100% {
    transform: scale(1);
    opacity: 1;
    }
}

.expandToFullScreen {
    animation: expandToFullScreen 1s forwards;
}

.contractFromFullScreen {
    animation: expandToFullScreen 1s reverse forwards;
}

.box {
    width: 80px;
    height: 80px;
    background: red;
    cursor: pointer;
    transition: 1s ease-in-out;
    margin: 0 auto;
    margin-top: 100px;
}

.fullscreen {
    width: 100vw;
    height: 100vh;
    position: absolute;
    top: 0 !important;
    left: 0 !important;
    background: red;
    transition: 1s ease-in-out;
}

.contract {
    width: 80px;
    height: 80px;
    position: absolute;
    top: 100px !important;
    left: 50% !important;
    transform: translateX(-50%);
    background: red;
    transition: 1s ease-in-out;
}

.modal {
    display: none;
    position: fixed;
    z-index: 1000;
    left: 0;
    top: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0,0,0,0.4);
}

.modal-content {
    background-color: #fefefe;
    margin: 15% auto;
    padding: 20px;
    border: 1px solid #888;
    width: 80%;
    max-width: 500px;
}

.close {
    color: #aaa;
    float: right;
    font-size: 28px;
    font-weight: bold;
    cursor: pointer;
}

.close:hover,
.close:focus {
    color: black;
    text-decoration: none;
    cursor: pointer;
}

#roomList div {
    margin: 10px 0;
    display: flex;
    justify-content: space-between;
    align-items: center;
}

#roomList select {
    margin-left: 10px;
    padding: 5px;
}</style>
    <style type="text/css">/* Modal */
.modal {
  display: none;
  position: fixed;
  z-index: 1;
  left: 0;
  top: 0;
  width: 100%;
  height: 100%;
  overflow: auto;
  background-color: rgba(0, 0, 0, 0.4);

}

/* 
.modal-content {
  background-color: #fefefe;
  margin: 15% auto;
  padding: 20px;
  border: 1px solid #888;
  width: 80%;
  max-width: 500px;
} */

#roomList div {
  margin: 10px 0;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

/* Style dla zakładek */
.config-tabs {
  display: flex;
  justify-content: center;
  margin-bottom: 15px;
}

.tab-button {
  padding: 8px 15px;
  margin: 0 5px;
  border: none;
  background-color: #f0f0f0;
  border-radius: 5px;
  cursor: pointer;
  transition: all 0.3s ease;
}

.tab-button.active {
  background-color: #474E65;
  color: white;
}

.tab-content {
  transition: all 0.3s ease;
}

/* Style dla edycji pokoi */
#roomList .room-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 15px;
  padding: 10px;
  border-radius: 8px;
  background-color: #f9f9f9;
}

#roomList .room-item input[type="text"] {
  padding: 5px;
  border: 1px solid #ddd;
  border-radius: 4px;
  margin-right: 10px;
  width: 40%;
}

#roomList .room-item select {
  padding: 5px;
  border: 1px solid #ddd;
  border-radius: 4px;
  width: 30%;
}

.modal-content {
    -webkit-box-shadow: 0px 0px 73px 15px rgba(66, 68, 90, 0.44);
    -moz-box-shadow: 0px 0px 73px 15px rgba(66, 68, 90, 0.44);
    box-shadow: 0px 0px 73px 15px rgba(66, 68, 90, 0.44);
    
    border-radius: 16px;
  background-color: #fefefeed;
  margin: 15% auto;
  padding: 20px;
  border: 1px solid #888;
  width: 80%;

  /* make opacity animation fade in  */
    animation: fadeIn 0.5s;

}


.close {
  color: #aaa;
  float: right;
  font-size: 28px;
  font-weight: bold;
}

.close:hover,
.close:focus {
  color: black;
  text-decoration: none;
  cursor: pointer;
}
hr {
  border: 1px solid #a5a5a510;
  margin-bottom: 20px;
}

button.cancel {
  background-color: #f44336;
  color: white;
  padding: 14px 20px;
  margin: 8px 0;
  border: none;
  cursor: pointer;
  width: 100%;
  border-radius: 12px;
  width: 25%;
}
button.confirm {
  background-color: #4CAF50;
  color: white;
  padding: 14px 20px;
  margin: 8px 0;
  border: none;
  cursor: pointer;
  width: 100%;
  border-radius: 12px;
  width: 25%;
  float: right;
}
button {
  background-color: #445A7D;
  color: white;
  padding: 14px 20px;
  margin: 8px 0;
  border: none;
  cursor: pointer;
  width: 100%;
  border-radius: 12px;
  width: 25%;
}</style>

    <!-- <script type="module" src="https://unpkg.com/ionicons@4.5.10-0/dist/ionicons/ionicons.esm.js"></script>
    <script nomodule="" src="https://unpkg.com/ionicons@4.5.10-0/dist/ionicons/ionicons.js"></script> -->

    <script src="//cdnjs.cloudflare.com/ajax/libs/qrcodejs/1.0.0/qrcode.min.js"></script>
    <script src="//cdn.jsdelivr.net/npm/dayjs@1/dayjs.min.js"></script>
    <script src="//cdn.jsdelivr.net/npm/dayjs@1/locale/pl.js"></script>
    <script>

          dayjs.locale("pl");
    
    </script>
    <link
      href="//fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700"
      rel="stylesheet"
      type="text/css"
    />
    <link
      href="//cdn.jsdelivr.net/npm/swiper@11/swiper-bundle.min.css"
      rel="stylesheet"
    />

    <!-- FAVICON GENERATION  -->
    <link
      href="data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZlcnNpb249IjEuMSIgeG1sbnM6eGxpbms9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkveGxpbmsiIHhtbG5zOnN2Z2pzPSJodHRwOi8vc3ZnanMuZGV2L3N2Z2pzIiB3aWR0aD0iNjY0LjAyIiBoZWlnaHQ9IjY2My43NyI+PHN2ZyBpZD0iU3ZnanNTdmcxMDAxIiBkYXRhLW5hbWU9ImZhdmljb25uIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCA2NjQuMDIgNjYzLjc3Ij4KICAKICA8cGF0aCBmaWxsPSIjZjM2ZDI0IiBkPSJNODkuMjksNTU4LjMzQy04NC40MiwzNzEuODgsOC4zMyw2Ny4wMSwyNTUuMTUsOS4wOWMyNTIuODYtNTkuMzQsNDczLjc3LDE4MS45NywzOTEuMzEsNDI4LjY5LTc5LjI5LDIzNy4yNy0zODUuNzQsMzA0LjU1LTU1Ny4xNywxMjAuNTZaTTEzOS4yNCw1MzkuNDFjMTU1LjIxLDE0My40Miw0MDcuNDQsNzIuNDEsNDY0LjY0LTEyOS45MUM2NjYuNiwxODcuNjIsNDUwLjY2LTEyLjk5LDIzMy45Miw2Ni41OCwzNy4wOCwxMzguODUtMTUuNiwzOTYuMzQsMTM5LjI0LDUzOS40MVoiPjwvcGF0aD4KICA8cGF0aCBmaWxsPSIjZjM2ZDI0IiBkPSJNMzcxLjc4LDExNC44OWMtMTYuMDMtMTUuNzMtNDMuNDYtMTQuMzMtNTcuNTEsMy4yOCw0LjU5LDUuOTksMTAuMDQsMTEuMTYsMTQuMzIsMTcuNDMsMTQuMTUsMjAuNzcsMjMuMTcsNTEuMjMsMjQuNiw3Ni4yOCw0LjQ4LDc4LjU1LTYzLjgzLDExNS44Ny04NS40NCwxODUuMzItMTMuMTIsNDIuMTctNi4wNSwxMDUuMjYsMTUuOTYsMTQzLjg1LDE0LjU3LDI1LjU2LDQ2LjA3LDMwLjkyLDY1LjM3LDcuNjEtMjMuNTEtMjMuMTUtMzYuODUtNTkuMjgtMzguODctOTIuMDQtNS41My04OS43Niw4NS42LTEzMC4xLDkxLjg3LTIxNy43NCwyLjI2LTMxLjYxLTYuODYtMTAwLjk3LTMwLjMtMTIzLjk5Wk00NjcuNzUsMTMxLjc2Yy03Ljc3LDEuMjEtMTYuNjIsNy4zNC0yMS42NywxMy4yNCwxOC45MywyMC4yOCwyOS43Nyw0OS4xMSwzMC45Niw3Ni44NywyLjIyLDUxLjQxLTQ2LjczLDg4LjE0LTY4LjE2LDEzMS42OS0xOS45NCw0MC41My0xOS4xNCw5MS4yNy0xLjI3LDEzMi41MSwxMi42OCwyOS4yNSw0Mi44MywzNy4zOCw2NS4zMiwxMy4zOC0xOC4yNS0yMC44OS0yOC4yOC00OC4wNy0yOS44OC03NS43OC0zLjY4LTYzLjkyLDYzLjY3LTEwMy41Nyw3OS4xOC0xNjQuNTEsOS4zOS0zNi45MiwyLjMyLTEzNi4yMy01NC40OC0xMjcuMzlaTTIzMi4wOSwxMzIuODFjLTguODMuNzYtMTcuNTEsNS4wMy0yMy42MywxMS4zMiw4LjQxLDEyLjcyLDE1LjMxLDI0LjQsMTkuMzQsMzkuMzMsOS45NiwzNi45LDUuMDIsNjQuMzktMTcuMDYsOTQuNzktMjUuMywzNC44My01OS42NCw2Mi4zMi03MS44MywxMDUuOTYtNy45LDI4LjI5LTcuMDksODUuNTMsOS4wNywxMTAuNzgsMTIuNDIsMTkuNDEsNDMuNDIsMjIuNDcsNTguMjgsNC40NC0xOS43My0yNC40NS0yNS40MS01Ni44Ny0yMi44OS04Ny43NCw0LjAxLTQ5LjA0LDcwLjcyLTk0LjAxLDg5LjY0LTE0NC4wOCwxMy4xOS0zNC45LDE1Ljk4LTEzOS42OC00MC45My0xMzQuODFaIj48L3BhdGg+CiAgPHBhdGggZmlsbD0iI2YzNmQyNCIgZD0iTTM3MS43OCwxMTQuODljMjMuNDQsMjMuMDEsMzIuNTYsOTIuMzgsMzAuMywxMjMuOTktNi4yNiw4Ny42NS05Ny40LDEyNy45OC05MS44NywyMTcuNzQsMi4wMiwzMi43NSwxNS4zNiw2OC44OCwzOC44Nyw5Mi4wNC0xOS4zMSwyMy4zMi01MC44LDE3Ljk1LTY1LjM3LTcuNjEtMjItMzguNTktMjkuMDgtMTAxLjY4LTE1Ljk2LTE0My44NSwyMS42MS02OS40Niw4OS45Mi0xMDYuNzcsODUuNDQtMTg1LjMyLTEuNDMtMjUuMDUtMTAuNDUtNTUuNTEtMjQuNi03Ni4yOC00LjI4LTYuMjgtOS43My0xMS40NC0xNC4zMi0xNy40MywxNC4wNC0xNy42MSw0MS40OC0xOS4wMSw1Ny41MS0zLjI4WiI+PC9wYXRoPgogIDxwYXRoIGZpbGw9IiNmMzZkMjQiIGQ9Ik0yMzIuMDksMTMyLjgxYzU2LjkxLTQuODcsNTQuMTEsOTkuOTEsNDAuOTMsMTM0LjgxLTE4LjkyLDUwLjA3LTg1LjY0LDk1LjA0LTg5LjY0LDE0NC4wOC0yLjUyLDMwLjg3LDMuMTUsNjMuMjksMjIuODksODcuNzQtMTQuODYsMTguMDMtNDUuODYsMTQuOTctNTguMjgtNC40NC0xNi4xNi0yNS4yNS0xNi45Ny04Mi40OS05LjA3LTExMC43OCwxMi4xOC00My42NCw0Ni41My03MS4xMyw3MS44My0xMDUuOTYsMjIuMDgtMzAuNCwyNy4wMi01Ny44OSwxNy4wNi05NC43OS00LjAzLTE0LjkzLTEwLjkzLTI2LjYxLTE5LjM0LTM5LjMzLDYuMTItNi4yOSwxNC44LTEwLjU2LDIzLjYzLTExLjMyWiI+PC9wYXRoPgogIDxwYXRoIGZpbGw9IiNmMzZkMjQiIGQ9Ik00NjcuNzUsMTMxLjc2YzU2LjgtOC44NCw2My44Nyw5MC40Nyw1NC40OCwxMjcuMzktMTUuNTEsNjAuOTQtODIuODYsMTAwLjU5LTc5LjE4LDE2NC41MSwxLjYsMjcuNzEsMTEuNjMsNTQuOSwyOS44OCw3NS43OC0yMi40OSwyNC01Mi42NCwxNS44Ny02NS4zMi0xMy4zOC0xNy44Ny00MS4yMy0xOC42OC05MS45OCwxLjI3LTEzMi41MSwyMS40My00My41NCw3MC4zOC04MC4yNyw2OC4xNi0xMzEuNjktMS4yLTI3Ljc2LTEyLjAzLTU2LjU5LTMwLjk2LTc2Ljg3LDUuMDUtNS45LDEzLjktMTIuMDQsMjEuNjctMTMuMjRaIj48L3BhdGg+Cjwvc3ZnPjxzdHlsZT5AbWVkaWEgKHByZWZlcnMtY29sb3Itc2NoZW1lOiBsaWdodCkgeyA6cm9vdCB7IGZpbHRlcjogbm9uZTsgfSB9CkBtZWRpYSAocHJlZmVycy1jb2xvci1zY2hlbWU6IGRhcmspIHsgOnJvb3QgeyBmaWx0ZXI6IG5vbmU7IH0gfQo8L3N0eWxlPjwvc3ZnPg=="
      rel="icon"
      type="image/svg+xml"
    />
    <!-- FAVICON GENERATION  -->
  </head>
  <body class="">
    <div id="loader" class="loader">
      <!-- <svg width="100" height="100" viewBox="0 0 100 100">
            <circle cx="50" cy="50" r="45"></circle>
        </svg> -->
    </div>

    <div class="container">
      <div id="thermostat-list">
       


      </div>

      <div class="imageBack_container">
        <svg xmlns="//www.w3.org/2000/svg" viewBox="0 0 88.02 64.13">
          <defs>
            <style>
              .cls-1 {
                fill: #fff;
                stroke-width: 0px;
              }
            </style>
          </defs>
          <path
            class="cls-1"
            d="m28.1,8.34c.26,0,.51-.1.71-.29l4.63-4.63v10.07c0,.42.27.8.67.94,1.41.5,3.33,1.19,3.33,1.91s-1.93,1.41-3.34,1.91c-2.19.78-4.66,1.66-4.66,3.79s2.48,3.01,4.67,3.79c1.41.5,3.33,1.19,3.33,1.9s-1.93,1.4-3.33,1.9c-2.19.78-4.67,1.66-4.67,3.79,0,1.9,1.98,2.81,3.83,3.49v6.01c0,.55.45,1,1,1s1-.45,1-1v-6.72c0-.42-.27-.8-.67-.94-1.18-.42-3.17-1.13-3.17-1.84s1.93-1.4,3.33-1.9c2.19-.78,4.67-1.66,4.67-3.79s-2.48-3.01-4.67-3.79c-1.41-.5-3.33-1.19-3.33-1.9s1.93-1.41,3.34-1.91c2.19-.78,4.66-1.66,4.66-3.79,0-1.92-2.01-2.82-4-3.55V3.41l4.62,4.62c.39.39,1.02.39,1.41,0s.39-1.02,0-1.41L35.14.29c-.09-.09-.2-.17-.33-.22-.24-.1-.52-.1-.76,0-.12.05-.23.12-.33.22l-6.33,6.34c-.39.39-.39,1.02,0,1.41.2.2.45.29.71.29Z"
          ></path>
          <path
            class="cls-1"
            d="m47.97,8.03l4.62-4.62v9.37c-1.99.73-4,1.64-4,3.55,0,2.13,2.48,3.01,4.66,3.79,1.41.5,3.34,1.19,3.34,1.91s-1.93,1.4-3.33,1.9c-2.19.78-4.67,1.66-4.67,3.79s2.48,3.01,4.67,3.79c1.41.5,3.33,1.19,3.33,1.9s-1.98,1.42-3.17,1.84c-.4.14-.67.52-.67.94v6.72c0,.55.45,1,1,1s1-.45,1-1v-6.01c1.85-.68,3.83-1.59,3.83-3.49,0-2.13-2.48-3.01-4.67-3.79-1.41-.5-3.33-1.19-3.33-1.9s1.93-1.4,3.33-1.9c2.19-.78,4.67-1.66,4.67-3.79s-2.48-3.01-4.66-3.79c-1.41-.5-3.34-1.19-3.34-1.91s1.93-1.41,3.33-1.91c.4-.14.67-.52.67-.94V3.41l4.63,4.63c.2.2.45.29.71.29s.51-.1.71-.29c.39-.39.39-1.02,0-1.41L54.3.29c-.09-.09-.2-.17-.33-.22-.24-.1-.52-.1-.76,0-.12.05-.23.12-.33.22l-6.33,6.32c-.39.39-.39,1.02,0,1.41s1.02.39,1.41,0Z"
          ></path>
          <g>
            <path
              class="cls-1"
              d="M50.43,62.22c-.48-.28-1.09-.11-1.37.37-.28.48-.11,1.09.37,1.37l.31.18h4l-3.31-1.91Z"
            ></path>
            <path
              class="cls-1"
              d="m.5,35.7l43.01,24.83c.15.09.33.13.5.13s.35-.04.5-.13l8.1-4.68,14.33,8.27h3.93c-.17-.13-.36-.25-.56-.36l-15.7-9.07,6.6-3.81,16.7,9.64c1.35.78,3.07,1.17,4.8,1.17s3.45-.39,4.8-1.17c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37-2.06,1.19-5.54,1.19-7.6,0l-15.7-9.07,24.31-14.03c.31-.18.5-.51.5-.87s-.19-.69-.5-.87l-22.3-12.88c-.48-.28-1.09-.11-1.37.37-.28.48-.11,1.09.37,1.37l20.8,12.01-41.01,23.68L3,34.83l20-11.55c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37L.5,33.97C.19,34.15,0,34.48,0,34.83s.19.69.5.87Z"
            ></path>
          </g>
          <g>
            <path
              class="cls-1"
              d="m17.7,55.57l14.83,8.56h4l-17.83-10.3c-.81-.47-1.28-1.06-1.28-1.62s.47-1.15,1.28-1.62c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37-1.47.85-2.28,2.04-2.28,3.35s.81,2.5,2.28,3.35Z"
            ></path>
            <path
              class="cls-1"
              d="m1.5,60.53c2.06-1.19,5.54-1.19,7.6,0l6.23,3.6h4l-9.23-5.33c-2.69-1.55-6.91-1.55-9.6,0-.48.28-.64.89-.37,1.37.28.48.89.64,1.37.37Z"
            ></path>
          </g>
        </svg>
      </div>

      <footer>
       
        <a href="#" class="noselect configButton">
          <div class="icon-inner">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              class="ionicon s-ion-icon"
              viewBox="0 0 512 512"
            >
              <title>Ustawienia</title>
              <path
                d="M456.7 242.27l-26.08-4.2a8 8 0 01-6.6-6.82c-.5-3.2-1-6.41-1.7-9.51a8.08 8.08 0 013.9-8.62l23.09-12.82a8.05 8.05 0 003.9-9.92l-4-11a7.94 7.94 0 00-9.4-5l-25.89 5a8 8 0 01-8.59-4.11q-2.25-4.2-4.8-8.41a8.16 8.16 0 01.7-9.52l17.29-19.94a8 8 0 00.3-10.62l-7.49-9a7.88 7.88 0 00-10.5-1.51l-22.69 13.63a8 8 0 01-9.39-.9c-2.4-2.11-4.9-4.21-7.4-6.22a8 8 0 01-2.5-9.11l9.4-24.75A8 8 0 00365 78.77l-10.2-5.91a8 8 0 00-10.39 2.21l-16.64 20.84a7.15 7.15 0 01-8.5 2.5s-5.6-2.3-9.8-3.71A8 8 0 01304 87l.4-26.45a8.07 8.07 0 00-6.6-8.42l-11.59-2a8.07 8.07 0 00-9.1 5.61l-8.6 25.05a8 8 0 01-7.79 5.41h-9.8a8.07 8.07 0 01-7.79-5.41l-8.6-25.05a8.07 8.07 0 00-9.1-5.61l-11.59 2a8.07 8.07 0 00-6.6 8.42l.4 26.45a8 8 0 01-5.49 7.71c-2.3.9-7.3 2.81-9.7 3.71-2.8 1-6.1.2-8.8-2.91l-16.51-20.34A8 8 0 00156.75 73l-10.2 5.91a7.94 7.94 0 00-3.3 10.09l9.4 24.75a8.06 8.06 0 01-2.5 9.11c-2.5 2-5 4.11-7.4 6.22a8 8 0 01-9.39.9L111 116.14a8 8 0 00-10.5 1.51l-7.49 9a8 8 0 00.3 10.62l17.29 19.94a8 8 0 01.7 9.52q-2.55 4-4.8 8.41a8.11 8.11 0 01-8.59 4.11l-25.89-5a8 8 0 00-9.4 5l-4 11a8.05 8.05 0 003.9 9.92L85.58 213a7.94 7.94 0 013.9 8.62c-.6 3.2-1.2 6.31-1.7 9.51a8.08 8.08 0 01-6.6 6.82l-26.08 4.2a8.09 8.09 0 00-7.1 7.92v11.72a7.86 7.86 0 007.1 7.92l26.08 4.2a8 8 0 016.6 6.82c.5 3.2 1 6.41 1.7 9.51a8.08 8.08 0 01-3.9 8.62L62.49 311.7a8.05 8.05 0 00-3.9 9.92l4 11a7.94 7.94 0 009.4 5l25.89-5a8 8 0 018.59 4.11q2.25 4.2 4.8 8.41a8.16 8.16 0 01-.7 9.52l-17.29 19.96a8 8 0 00-.3 10.62l7.49 9a7.88 7.88 0 0010.5 1.51l22.69-13.63a8 8 0 019.39.9c2.4 2.11 4.9 4.21 7.4 6.22a8 8 0 012.5 9.11l-9.4 24.75a8 8 0 003.3 10.12l10.2 5.91a8 8 0 0010.39-2.21l16.79-20.64c2.1-2.6 5.5-3.7 8.2-2.6 3.4 1.4 5.7 2.2 9.9 3.61a8 8 0 015.49 7.71l-.4 26.45a8.07 8.07 0 006.6 8.42l11.59 2a8.07 8.07 0 009.1-5.61l8.6-25a8 8 0 017.79-5.41h9.8a8.07 8.07 0 017.79 5.41l8.6 25a8.07 8.07 0 009.1 5.61l11.59-2a8.07 8.07 0 006.6-8.42l-.4-26.45a8 8 0 015.49-7.71c4.2-1.41 7-2.51 9.6-3.51s5.8-1 8.3 2.1l17 20.94A8 8 0 00355 439l10.2-5.91a7.93 7.93 0 003.3-10.12l-9.4-24.75a8.08 8.08 0 012.5-9.12c2.5-2 5-4.1 7.4-6.21a8 8 0 019.39-.9L401 395.66a8 8 0 0010.5-1.51l7.49-9a8 8 0 00-.3-10.62l-17.29-19.94a8 8 0 01-.7-9.52q2.55-4.05 4.8-8.41a8.11 8.11 0 018.59-4.11l25.89 5a8 8 0 009.4-5l4-11a8.05 8.05 0 00-3.9-9.92l-23.09-12.82a7.94 7.94 0 01-3.9-8.62c.6-3.2 1.2-6.31 1.7-9.51a8.08 8.08 0 016.6-6.82l26.08-4.2a8.09 8.09 0 007.1-7.92V250a8.25 8.25 0 00-7.27-7.73zM256 112a143.82 143.82 0 01139.38 108.12A16 16 0 01379.85 240H274.61a16 16 0 01-13.91-8.09l-52.1-91.71a16 16 0 019.85-23.39A146.94 146.94 0 01256 112zM112 256a144 144 0 0143.65-103.41 16 16 0 0125.17 3.47L233.06 248a16 16 0 010 15.87l-52.67 91.7a16 16 0 01-25.18 3.36A143.94 143.94 0 01112 256zm144 144a146.9 146.9 0 01-38.19-4.95 16 16 0 01-9.76-23.44l52.58-91.55a16 16 0 0113.88-8H379.9a16 16 0 0115.52 19.88A143.84 143.84 0 01256 400z"
              ></path>
            </svg>
          </div>
        </a>

        <a href="#" class="noselect usegaz" id="useGazButton">
          <div class="icon-inner">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              class="ionicon s-ion-icon"
              viewBox="0 0 512 512"
            >
              <title>Włącz piecyk gazowy</title>
              <rect
          x="48"
          y="48"
          width="416"
          height="416"
          rx="96"
          stroke-linejoin="round"
          class="ionicon-fill-none ionicon-stroke-width"
              ></rect>
              <path
          d="M388.94 151.56c-24.46-22.28-68.72-51.4-132.94-51.4s-108.48 29.12-132.94 51.4a34.66 34.66 0 00-3.06 48.08l33.32 39.21a26.07 26.07 0 0033.6 5.21c15.92-9.83 40.91-21.64 69.1-21.64s53.18 11.81 69.1 21.64a26.07 26.07 0 0033.6-5.21L392 199.64a34.66 34.66 0 00-3.06-48.08z"
          stroke-linejoin="round"
          class="ionicon-fill-none ionicon-stroke-width"
              ></path>
            </svg>
          </div>
        </a>


        <a href="#" class="noselect a_fire">
          <div class="icon-inner">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              class="ionicon s-ion-icon"
              viewBox="0 0 512 512"
            >
              <title>Kominek działa</title>
              <path
                d="M273.38 368.33c-.81-9.23-8.86-16.44-18.55-16.44a18.63 18.63 0 00-18.2 14.11l-18.2 88.38a35.75 35.75 0 00-.93 7.88c0 19.93 16.68 33.78 37.33 33.78s37.34-13.85 37.34-33.78a36.16 36.16 0 00-1.29-9.45zM411.05 407.86a41.78 41.78 0 00-5.95-4.35l-69.53-48.59c-6.77-4.25-14-4.13-19.25 1a13.53 13.53 0 00-2 17.2l52.5 69a36.57 36.57 0 004 4.69c9.1 10.17 29.28 10.73 40.37 0 10.94-10.91 10.81-30.35-.14-38.95zM463.3 335.88h-70.42c-4.55 0-8.88 3.35-8.88 8.16s2.95 7.85 6.92 9.16l66.43 20.55C467 376 480 367.4 480 356.66c0-13.66-7.6-20.78-16.7-20.78zM128.22 344c0-5-4.55-8.16-9.45-8.16H48.35c-8.87 0-16.35 9.58-16.35 20.32S44.62 376 54.3 373.75l67.43-20.55c4.39-1.31 6.49-4.2 6.49-9.2zM176.55 355L107 403.38a32.34 32.34 0 00-6 4.35 26.33 26.33 0 000 38.56 29.39 29.39 0 0040.36 0 30.75 30.75 0 004-4.68L197.9 373c3.5-5.58 2.92-12.49-2-17.06a15 15 0 00-19.35-.94z"
              ></path>
              <path
                d="M368 125.09c0-55.7-49.22-89.16-94.2-93.06a8 8 0 00-8.69 7.71c-3.12 76-109.38 110.69-119.87 178.68-7 45.4 16.65 72.37 34.31 85.78a15.94 15.94 0 0023.93-5.62c22.5-45.86 74.33-63.49 109.72-85.94 42.88-27.2 54.8-56.44 54.8-87.55z"
                stroke-linecap="round"
                stroke-miterlimit="10"
                class="ionicon-fill-none ionicon-stroke-width"
              ></path>
              <path
                d="M240 95.7c-12.82-23-30.51-31-59.16-31.63a3.86 3.86 0 00-4.06 3.51c-.95 34.22-32.78 44.85-32.78 73.15 0 19.37 5.39 29 17.16 39.28M352.45 178.76c8.6 14.31 15.55 30.08 15.55 48 0 52.52-42.47 93.1-94.86 93.1a94.42 94.42 0 01-65.14-26"
                stroke-linecap="round"
                stroke-miterlimit="10"
                class="ionicon-fill-none ionicon-stroke-width"
              ></path>
            </svg>
          </div>
        </a>

        <a href="#" class="noselect a_weather">
          <img src="////openweathermap.org/img/wn/04d@2x.png" alt="Prognoza pogody" />
        </a>
        <a href="#" class="noselect a_qrcode">
          <div class="icon-inner">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              class="ionicon s-ion-icon"
              viewBox="0 0 512 512"
            >
              <title>Qr Code</title>
              <rect x="336" y="336" width="80" height="80" rx="8" ry="8"></rect>
              <rect x="272" y="272" width="64" height="64" rx="8" ry="8"></rect>
              <rect x="416" y="416" width="64" height="64" rx="8" ry="8"></rect>
              <rect x="432" y="272" width="48" height="48" rx="8" ry="8"></rect>
              <rect x="272" y="432" width="48" height="48" rx="8" ry="8"></rect>
              <rect x="336" y="96" width="80" height="80" rx="8" ry="8"></rect>
              <rect
                x="288"
                y="48"
                width="176"
                height="176"
                rx="16"
                ry="16"
                stroke-linecap="round"
                stroke-linejoin="round"
                class="ionicon-fill-none ionicon-stroke-width"
              ></rect>
              <rect x="96" y="96" width="80" height="80" rx="8" ry="8"></rect>
              <rect
                x="48"
                y="48"
                width="176"
                height="176"
                rx="16"
                ry="16"
                stroke-linecap="round"
                stroke-linejoin="round"
                class="ionicon-fill-none ionicon-stroke-width"
              ></rect>
              <rect x="96" y="336" width="80" height="80" rx="8" ry="8"></rect>
              <rect
                x="48"
                y="288"
                width="176"
                height="176"
                rx="16"
                ry="16"
                stroke-linecap="round"
                stroke-linejoin="round"
                class="ionicon-fill-none ionicon-stroke-width"
              ></rect>
            </svg>
          </div>
        </a>

        <div class="right">


          

          <p class="heatState">
            <svg
              class="iconHeat"
              version="1.1"
              xmlns="//www.w3.org/2000/svg"
              xmlns:xlink="//www.w3.org/1999/xlink"
              x="0px"
              y="0px"
              width="980px"
              height="778.2px"
              viewBox="0 0 980 778.2"
              style="overflow: visible; enable-background: new 0 0 980 778.2"
              xml:space="preserve"
            >
              <style type="text/css">
                .st0 {
                  fill: #fd8300;
                }
              </style>
              <defs></defs>
              <g>
                <g
                  transform="translate(0.000000,511.000000) scale(0.100000,-0.100000)"
                >
                  <path
                    class="st0"
                    d="M8845.3,4724.7c480.4-579.6,456.6-492.3,206.5-766.2c-369.2-397-500.2-913.1-369.2-1429.2
                   c75.4-301.7,246.2-678.9,412.9-921.1c345.3-504.2,619.3-1238.7,686.8-1854.1c71.5-623.3-75.4-1250.6-401-1742.9
                   c-166.8-246.2-611.4-698.8-670.9-674.9c-39.7,11.9-718.6,889.3-718.6,929c0,11.9,43.7,55.6,91.3,99.3
                   c170.7,131,381.1,484.4,448.6,750.4c131,516.1-15.9,1079.9-484.4,1869.9c-536,905.2-730.5,1679.4-603.4,2386
                   c75.4,412.9,202.5,726.5,408.9,1044.1c166.8,250.1,571.7,694.8,635.2,694.8C8511.9,5109.8,8670.7,4935.1,8845.3,4724.7z"
                  />
                  <path
                    class="st0"
                    d="M5184.9,4665.1l377.2-444.6l-198.5-222.3c-285.8-317.6-397-579.6-420.8-952.8
                   c-27.8-504.2,87.3-861.5,492.3-1516.6c258-420.9,412.8-762.3,539.9-1206.9c71.5-254.1,91.3-412.9,91.3-849.6
                   c0-619.3-71.5-913.1-325.6-1349.8c-150.9-254.1-639.2-794-718.6-794c-23.8,0-206.4,210.4-401,472.5l-357.3,468.5l178.7,174.7
                   c99.3,95.3,226.3,274,285.9,393.1c99.2,198.5,107.2,258.1,103.2,674.9c0,555.8-71.5,770.2-492.3,1468.9
                   c-488.3,821.8-663,1409.4-623.3,2124C3751.7,3780,3989.9,4308,4490.2,4840c138.9,146.9,270,270,289.8,270
                   C4795.9,5109.8,4982.4,4907.3,5184.9,4665.1z"
                  />
                  <path
                    class="st0"
                    d="M1441.1,4704.8c186.6-222.3,353.3-428.8,369.2-456.6c23.8-39.7-19.9-115.1-139-242.2
                   c-238.2-258.1-337.5-428.8-412.9-726.5c-123.1-484.3,7.9-1000.5,440.7-1707.1c484.4-794,667-1365.7,671-2100.2
                   c0-532-67.5-833.7-285.9-1250.6c-142.9-274-682.9-893.3-778.2-893.3c-35.7,0-595.5,702.7-706.7,893.3
                   c-15.9,27.8,43.7,123,154.8,238.2c337.5,353.4,464.5,813.9,373.2,1349.8c-63.5,365.3-186.6,655.1-512.1,1202.9
                   C158.8,1782.9,0,2283.1,0,2946.1c0,591.5,146.9,1056,496.3,1556.3c150.9,222.3,516.1,607.4,571.7,607.4
                   C1087.8,5109.8,1258.5,4927.2,1441.1,4704.8z"
                  />
                </g>
              </g>
            </svg>
          </p>

          <p id="ws-status">
            <svg
              class="ws_connect_icon"
              xmlns="http://www.w3.org/2000/svg"
              version="1.1"
              viewBox="0 0 63.54 63.65"
            >
              <path
                class="st0_wsConnect"
                d="M63.54,31.77c.23,22.22-22.28,37.65-43.12,29.62-2.68-1.03-5.15-2.52-7.51-4.12l-6.41,6.38-.48-.1-6.01-6.07,6.45-6.67C-6.35,33.61,1.32,8.71,21.57,1.77,24.8.67,28.1.27,31.49,0c.16-.04.46.2.46.28v8.61l-.32.26c-7.67.08-14.84,4.13-18.9,10.57-6.3,9.99-3.83,23.32,5.65,30.31,13.66,10.07,33.21,2.1,35.91-14.59.17-1.04.09-2.32.25-3.2.04-.21.1-.36.29-.48h8.71Z"
              />
              <path
                class="st1_wsConnect"
                d="M51.03,6.33c.17-.04.31-.06.45.07l5.84,5.84.07.45-13.1,13.15c.27,1.29.79,2.47,1,3.78,1.91,11.71-11.12,20.14-20.98,13.52-12.37-8.31-4.28-28.02,11.05-24.45.55.13,2.2.87,2.52.74l13.15-13.1ZM35.14,28.59c-2.31-2.31-6.57-1.37-7.53,1.8-1.47,4.81,4.58,8.11,7.79,4.34,1.45-1.7,1.31-4.58-.26-6.15Z"
              />
            </svg>
          </p>

          <p class="console noselect"></p>
        </div>
      </footer>
    </div>

    <!-- Modal -->
    <div id="configModal" class="modal">
      <div class="modal-content">
        <span class="close no-select">&times;</span>
        <h2 class="no-select">Konfiguracja</h2>
        <br/>

        <!-- Zakładki -->
        <div class="config-tabs">
          <button class="tab-button active" data-tab="rooms">POKOJE</button>
          <button class="tab-button" data-tab="settings">USTAWIENIA</button>
        </div>

        <hr/>
        <br/>

        <!-- Zawartość zakładki POKOJE -->
        <div class="tab-content" id="rooms-tab">
          <div id="roomList"></div>
        </div>

        <!-- Zawartość zakładki USTAWIENIA -->
        <div class="tab-content" id="settings-tab" style="display: none;">
          <div class="settings-container">
            <h3>Ustawienia ogólne</h3>
            <div class="setting-item">
              <label>Tryb pracy:</label>
              <select id="workMode">
                <option value="auto">Automatyczny</option>
                <option value="manual">Manualny</option>
              </select>
            </div>
          </div>
        </div>

        <br/>
        <hr/>
        <br/>

        <button class="" id="wifiConfig">Wifi settings</button>
        <button class="confirm" id="saveConfig">Zapisz</button>
      </div>
    </div>




    <script src="src/app.js" type="text/javascript"></script>

  </body>
</html>


)rawliteral";
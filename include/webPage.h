#include <Arduino.h>

const char webpage[] PROGMEM = R"rawliteral(

<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,minimum-scale=1" />
  <meta name="theme-color" content="#474E65" />
  <meta name="mobile-web-app-capable" content="yes" />
  <meta name="application-name" content="Netatmo Relay" />
  <title>Netatmo Relay</title>
  <link href="https://unpkg.com/tailwindcss@^2/dist/tailwind.min.css" rel="stylesheet" />
  <link href="https://fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700"
    rel="stylesheet" type="text/css" />
  <script type="text/javascript" src="https://unpkg.co/gsap@3/dist/gsap.min.js"></script>
  <script type="text/javascript" src="https://unpkg.com/gsap@3/dist/Draggable.min.js"></script>
  <script src="https://code.jquery.com/jquery-3.6.0.min.js"
    integrity="sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4=" crossorigin="anonymous"></script>

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

    body::before {
      content: "";
      position: absolute;
      opacity: 0;
      top: 0px;
      right: 0px;
      bottom: 0px;
      left: 0px;
      transition: all ease 0.5s;
      background-image: -webkit-linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);
      background-image: -moz-linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);
      background-image: -o-linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);
      background-image: linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);

      background-position: 100px;
    }

    body.orange::before {
      content: "";
      position: absolute;
      top: 0px;
      right: 0px;
      bottom: 0px;
      left: 0px;
      opacity: 1;
      transition: all ease 0.5s;
      background-image: -webkit-linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);
      background-image: -moz-linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);
      background-image: -o-linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);
      background-image: linear-gradient(#fd8000,
          #fe8a00 20%,
          #fe9300 40%,
          #ff9c00 60%,
          orange 80%,
          #ffae00);

      background-position: 100px;
    }

    body.connected {
      border: 1rem solid rgba(0, 0, 0, 0.1);
    }

    body {
      /*   background-image: transparent;*/
      border: 0rem solid rgba(0, 0, 0, 0);
      background-image: linear-gradient(rgba(5, 5, 5, 0.2),
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2) 0%,
          rgba(5, 5, 5, 0.2));

      transition: all ease 0.5s;

      /*	background: #34495e;*/
      font-family: "Open Sans";
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

    .checkbox-container .handIcon {
      position: absolute;
      /* color: rgba(250, 144, 3, 1.000); */
      color: #fff;
      margin-left: 83px;
      transition: all ease-in-out 0.6s;
      opacity: 1;
      z-index: 900000;
    }

    .checkbox-container .handIcon.disabled {
      opacity: 0;
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
      content: " ";
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

    .checkbox-container input:checked+label+.active-circle {
      transform: translate(-50%, -50%) scale(15);
    }

    .checkbox-container input:checked+label::after {
      transition: transform 0.5s ease-out;
      transform: translateX(calc(100% + 0.5px));
    }

    .checkbox-container input:checked+label::after {
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
    .checkbox-container.purple input:checked+label {
      background: linear-gradient(to bottom,
          rgba(255, 255, 255, 0.15) 0%,
          rgba(0, 0, 0, 0.15) 100%),
        radial-gradient(at top center,
          rgba(255, 255, 255, 0.4) 0%,
          rgba(0, 0, 0, 0.4) 120%) #989898;
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

    footer a:active {
      background-color: rgba(185, 66, 67, 1.000);

    }

    footer a.usegaz {
      padding: 10px;
      padding-left: 15px;

    }

    footer a.usegaz.active {

      background-color: rgba(185, 66, 67, 1.000);

    }

    footer a.usegaz svg {
      width: 24px;

    }

    body {
      margin-top: 120px;
    }

    .containerouter {
      scale: 80%;
      /* height: 100%; */
      position: absolute;
      overflow: hidden;
      margin-bottom: 400px;
      margin-top: 20px;
      border-radius: 20px;
      box-shadow: 0px 0px 122px rgba(10, 10, 10, 0.2);
      border: solid rgba(20, 20, 20, 0.3);
    }

    .containerouter .container.disabled {

      opacity: 0.4;
    }

    @media (max-height: 980px) {
      .containerouter {
        margin-top: 40px;
        scale: 60%;

      }
    }

    footer .console {
      z-index: 200;
      position: absolute;
      right: 20px;
      top: 8px;
    }

    footer .iconHeat {
      width: 30px;
      height: 30px;
      position: absolute;
      right: 120px;
      top: 15px;
      opacity: 0.3;
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

    .imageBack_container {
      position: absolute;
      z-index: 0;
      right: -24%;
      bottom: 0px;
      width: 80%;
      overflow: hidden;
      opacity: 0.05;
    }

    .noselect {
      -webkit-touch-callout: none;
      /* iOS Safari */
      -webkit-user-select: none;
      /* Safari */
      -khtml-user-select: none;
      /* Konqueror HTML */
      -moz-user-select: none;
      /* Old versions of Firefox */
      -ms-user-select: none;
      /* Internet Explorer/Edge */
      user-select: none;
      /* Non-prefixed version, currently
                                  supported by Chrome, Edge, Opera and Firefox */
    }
  </style>
</head>

<html>

<body class="">
  <!--  <div class="container">  -->
  <div class="imageBack_container">
    <?xml version="1.0" encoding="UTF-8"?>
    <svg id="Tryb_izolacji" data-name="Tryb izolacji" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 88.02 64.13">
      <defs>
        <style>
          .cls-1 {
            fill: #fff;
            stroke-width: 0px;
          }
        </style>
      </defs>
      <path class="cls-1"
        d="m28.1,8.34c.26,0,.51-.1.71-.29l4.63-4.63v10.07c0,.42.27.8.67.94,1.41.5,3.33,1.19,3.33,1.91s-1.93,1.41-3.34,1.91c-2.19.78-4.66,1.66-4.66,3.79s2.48,3.01,4.67,3.79c1.41.5,3.33,1.19,3.33,1.9s-1.93,1.4-3.33,1.9c-2.19.78-4.67,1.66-4.67,3.79,0,1.9,1.98,2.81,3.83,3.49v6.01c0,.55.45,1,1,1s1-.45,1-1v-6.72c0-.42-.27-.8-.67-.94-1.18-.42-3.17-1.13-3.17-1.84s1.93-1.4,3.33-1.9c2.19-.78,4.67-1.66,4.67-3.79s-2.48-3.01-4.67-3.79c-1.41-.5-3.33-1.19-3.33-1.9s1.93-1.41,3.34-1.91c2.19-.78,4.66-1.66,4.66-3.79,0-1.92-2.01-2.82-4-3.55V3.41l4.62,4.62c.39.39,1.02.39,1.41,0s.39-1.02,0-1.41L35.14.29c-.09-.09-.2-.17-.33-.22-.24-.1-.52-.1-.76,0-.12.05-.23.12-.33.22l-6.33,6.34c-.39.39-.39,1.02,0,1.41.2.2.45.29.71.29Z" />
      <path class="cls-1"
        d="m47.97,8.03l4.62-4.62v9.37c-1.99.73-4,1.64-4,3.55,0,2.13,2.48,3.01,4.66,3.79,1.41.5,3.34,1.19,3.34,1.91s-1.93,1.4-3.33,1.9c-2.19.78-4.67,1.66-4.67,3.79s2.48,3.01,4.67,3.79c1.41.5,3.33,1.19,3.33,1.9s-1.98,1.42-3.17,1.84c-.4.14-.67.52-.67.94v6.72c0,.55.45,1,1,1s1-.45,1-1v-6.01c1.85-.68,3.83-1.59,3.83-3.49,0-2.13-2.48-3.01-4.67-3.79-1.41-.5-3.33-1.19-3.33-1.9s1.93-1.4,3.33-1.9c2.19-.78,4.67-1.66,4.67-3.79s-2.48-3.01-4.66-3.79c-1.41-.5-3.34-1.19-3.34-1.91s1.93-1.41,3.33-1.91c.4-.14.67-.52.67-.94V3.41l4.63,4.63c.2.2.45.29.71.29s.51-.1.71-.29c.39-.39.39-1.02,0-1.41L54.3.29c-.09-.09-.2-.17-.33-.22-.24-.1-.52-.1-.76,0-.12.05-.23.12-.33.22l-6.33,6.32c-.39.39-.39,1.02,0,1.41s1.02.39,1.41,0Z" />
      <g>
        <path class="cls-1" d="m50.43,62.22c-.48-.28-1.09-.11-1.37.37-.28.48-.11,1.09.37,1.37l.31.18h4l-3.31-1.91Z" />
        <path class="cls-1"
          d="m.5,35.7l43.01,24.83c.15.09.33.13.5.13s.35-.04.5-.13l8.1-4.68,14.33,8.27h3.93c-.17-.13-.36-.25-.56-.36l-15.7-9.07,6.6-3.81,16.7,9.64c1.35.78,3.07,1.17,4.8,1.17s3.45-.39,4.8-1.17c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37-2.06,1.19-5.54,1.19-7.6,0l-15.7-9.07,24.31-14.03c.31-.18.5-.51.5-.87s-.19-.69-.5-.87l-22.3-12.88c-.48-.28-1.09-.11-1.37.37-.28.48-.11,1.09.37,1.37l20.8,12.01-41.01,23.68L3,34.83l20-11.55c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37L.5,33.97C.19,34.15,0,34.48,0,34.83s.19.69.5.87Z" />
      </g>
      <g>
        <path class="cls-1"
          d="m17.7,55.57l14.83,8.56h4l-17.83-10.3c-.81-.47-1.28-1.06-1.28-1.62s.47-1.15,1.28-1.62c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37-1.47.85-2.28,2.04-2.28,3.35s.81,2.5,2.28,3.35Z" />
        <path class="cls-1"
          d="m1.5,60.53c2.06-1.19,5.54-1.19,7.6,0l6.23,3.6h4l-9.23-5.33c-2.69-1.55-6.91-1.55-9.6,0-.48.28-.64.89-.37,1.37.28.48.89.64,1.37.37Z" />
      </g>
    </svg>

  </div>
  <div class="containerouter">


    <div class="container">
      <h3 class="noselect">Gościnny</h3>
      <div class="checkbox-container purple">
        <ion-icon class="handIcon disabled" name="hand-right-outline"></ion-icon>
        <input type="checkbox" class="switch" id="pin_0" />
        <label for="pin_0"></label>
        <div class="active-circle"></div>
      </div>
    </div>

    <div class="container">
      <h3 class="noselect">Sypialnia</h3>
      <div class="checkbox-container purple">
        <ion-icon class="handIcon disabled" name="hand-right-outline"></ion-icon>
        <input type="checkbox" class="switch" id="pin_1" />
        <label for="pin_1"></label>
        <div class="active-circle"></div>
      </div>
    </div>

    <div class="container">
      <h3 class="noselect">Kuchnia</h3>
      <div class="checkbox-container purple">
        <ion-icon class="handIcon disabled" name="hand-right-outline"></ion-icon>
        <input type="checkbox" class="switch" id="pin_2" />
        <label for="pin_2"></label>
        <div class="active-circle"></div>
      </div>
    </div>

    <div class="container">
      <h3 class="noselect">Łazienka</h3>
      <div class="checkbox-container purple">
        <ion-icon class="handIcon disabled" name="hand-right-outline"></ion-icon>
        <input type="checkbox" class="switch" id="pin_3" />
        <label for="pin_3"></label>
        <div class="active-circle"></div>
      </div>
    </div>

    <div class="container disabled">
      <h3 class="noselect">Klatka schodowa</h3>
      <div class="checkbox-container purple">
        <ion-icon class="handIcon disabled" name="hand-right-outline"></ion-icon>
        <input type="checkbox" class="switch" disabled id="pin_4" />
        <label for="pin_4"></label>
        <div class="active-circle"></div>
      </div>
    </div>
    <div class="container disabled">
      <h3 class="noselect">Przedpokój</h3>
      <div class="checkbox-container purple">
        <ion-icon class="handIcon disabled" name="hand-right-outline"></ion-icon>
        <input type="checkbox" class="switch" id="pin_0" disabled />

        <label for="pin_0"></label>
        <div class="active-circle"></div>
      </div>
    </div>



  </div>



  <!--    </div>-->
  <footer>
    <a href="config">
      <ion-icon name="cog-outline" size="large"></ion-icon>
    </a>
    <a href="#" class="usegaz">
      <?xml version="1.0" encoding="UTF-8"?>
      <svg id="icons" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18.88 26.6">
        <defs>
          <style>
            .cls-1 {
              fill: #fff;
              stroke-width: 0px;
            }
          </style>
        </defs>
        <path class="cls-1"
          d="m2.62,26.6c-.27,0-.49-.22-.49-.49v-2.4l-1.67-.13c-.26-.02-.46-.24-.46-.49V1.27c0-.25.18-.46.43-.49l2.77-.37c.86-.11,1.75-.21,2.73-.28,1.2-.09,2.39-.14,3.52-.14s2.31.05,3.52.14c.98.07,1.87.16,2.73.28l2.77.37c.24.03.43.24.43.49v21.81c0,.26-.2.47-.45.49l-1.67.13v2.4c0,.27-.22.49-.49.49H2.62Zm13.14-.99v-1.83c-2.11.16-4.23.24-6.32.24s-4.22-.08-6.33-.24v1.83h12.65Zm-12.9-2.83c.97.08,1.94.13,2.92.17,1.33.06,2.52.09,3.66.09s2.33-.03,3.66-.09c.97-.04,1.95-.1,2.92-.17l1.87-.15v-3.23H7.97s0,.05,0,.07c0,1.44-1.17,2.61-2.61,2.61s-2.61-1.17-2.61-2.61c0-.04,0-.08,0-.11H.99v3.28l1.87.15Zm1.06-4.05c-.12.23-.18.48-.18.74,0,.89.73,1.62,1.62,1.62s1.62-.73,1.62-1.62-.73-1.62-1.62-1.62c-.26,0-.51.06-.74.18l1.09,1.09c.19.19.19.51,0,.7-.09.09-.22.14-.35.14s-.26-.05-.35-.14l-1.09-1.09Zm-.96-.25c.13-.32.31-.6.54-.83,0,0,.01-.02.02-.03,0,0,.02-.02.03-.02.49-.48,1.13-.74,1.82-.74,1.03,0,1.98.63,2.39,1.58h10.14V1.7l-2.34-.31c-.37-.05-.74-.09-1.11-.13-1.69-.18-3.37-.28-5-.28s-3.32.09-5,.28c-.38.04-.74.08-1.11.13l-2.34.31v16.77h1.97Z" />
        <path class="cls-1"
          d="m18.42.98l-1.81-.24s0,0,0,0l-.96-.13c-.9-.12-1.81-.21-2.71-.28-1.17-.09-2.33-.13-3.5-.13s-2.34.05-3.5.13c-.91.07-1.81.16-2.71.28l-.96.13s0,0,0,0l-1.81.24c-.15.02-.26.14-.26.29v21.81c0,.15.12.28.27.29l1.85.15v2.58c0,.16.13.29.29.29h13.64c.16,0,.29-.13.29-.29v-2.58l1.86-.15c.15-.01.27-.14.27-.29V1.27c0-.15-.11-.27-.26-.29Zm-2.39,21.99c-.97.08-1.95.13-2.93.17-1.22.05-2.44.09-3.67.09s-2.45-.03-3.67-.09c-.98-.04-1.95-.1-2.93-.17l-2.06-.16v-3.66h2.2c-.01.1-.03.21-.03.31,0,1.33,1.08,2.41,2.41,2.41s2.41-1.08,2.41-2.41c0-.09-.02-.18-.03-.27h10.34v3.62l-2.06.16Zm-12.15-4.57l1.27,1.27c.06.06.13.09.21.09s.15-.03.21-.09c.11-.12.11-.3,0-.42l-1.27-1.27c.3-.22.66-.35,1.06-.35,1,0,1.82.82,1.82,1.82s-.82,1.82-1.82,1.82-1.82-.82-1.82-1.82c0-.4.13-.76.35-1.06ZM.79,1.53l2.51-.33c.37-.05.74-.09,1.11-.13,1.67-.18,3.35-.28,5.03-.28s3.36.1,5.03.28c.37.04.74.08,1.11.13l2.51.33v17.1H7.62c-.34-.92-1.22-1.58-2.25-1.58-.66,0-1.26.27-1.7.7,0,0,0,0,0,0,0,0,0,0,0,0-.25.25-.44.56-.56.91H.79V1.53Zm2.13,24.28v-2.24c2.17.17,4.35.26,6.53.26s4.36-.09,6.53-.26v2.24H2.91Z" />
        <path class="cls-1"
          d="m11.76,21.61c-.27,0-.49-.22-.49-.49s.22-.49.49-.49h.64c.27,0,.49.22.49.49s-.22.49-.49.49h-.64Z" />
        <path class="cls-1"
          d="m12.39,20.83h-.64c-.16,0-.29.13-.29.29s.13.29.29.29h.64c.16,0,.29-.13.29-.29s-.13-.29-.29-.29Z" />
        <path class="cls-1"
          d="m14.2,21.61c-.27,0-.49-.22-.49-.49s.22-.49.49-.49h.64c.27,0,.49.22.49.49s-.22.49-.49.49h-.64Z" />
        <path class="cls-1"
          d="m14.84,20.83h-.64c-.16,0-.29.13-.29.29s.13.29.29.29h.64c.16,0,.29-.13.29-.29s-.13-.29-.29-.29Z" />
        <path class="cls-1"
          d="m9.44,15.46c2.17,0,3.86-1.06,3.86-2.41,0-.79-.57-1.48-1.55-1.93-.05.16-.1.31-.16.46.8.36,1.29.91,1.29,1.47,0,1.03-1.58,1.91-3.45,1.91s-3.45-.87-3.45-1.91c0-.56.49-1.11,1.29-1.47-.06-.15-.11-.3-.16-.46-.98.45-1.55,1.15-1.55,1.93,0,1.35,1.7,2.41,3.86,2.41Z" />
        <path class="cls-1"
          d="m8.55,12.31c-.08-.26-.13-.53-.17-.8-.07-.48-.07-.96-.04-1.44.07-1.08.41-1.9,1.07-2.75,1.55,1.23,1.31,4.04.71,5.67,1.29-1.51,1.74-3.72,1.6-5.66-.07-1.02-.32-2.04-.72-2.98-.19-.46-.92-1.88-1.57-1.88-.05,0-.11,0-.16.03-.73.29-1.27,1.46-1.54,2.14-.37.93-.56,1.92-.6,2.92-.07,1.96.38,3.88,1.6,5.44.08-.13-.16-.54-.2-.68Z" />
      </svg>
    </a>

    <p>
      <svg class="iconHeat" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
        x="0px" y="0px" width="980px" height="778.2px" viewBox="0 0 980 778.2"
        style="overflow: visible; enable-background: new 0 0 980 778.2" xml:space="preserve">
        <style type="text/css">
          .st0 {
            fill: #fd8300;
          }
        </style>
        <defs></defs>
        <g>
          <g transform="translate(0.000000,511.000000) scale(0.100000,-0.100000)">
            <path class="st0"
              d="M8845.3,4724.7c480.4-579.6,456.6-492.3,206.5-766.2c-369.2-397-500.2-913.1-369.2-1429.2
    c75.4-301.7,246.2-678.9,412.9-921.1c345.3-504.2,619.3-1238.7,686.8-1854.1c71.5-623.3-75.4-1250.6-401-1742.9
    c-166.8-246.2-611.4-698.8-670.9-674.9c-39.7,11.9-718.6,889.3-718.6,929c0,11.9,43.7,55.6,91.3,99.3
    c170.7,131,381.1,484.4,448.6,750.4c131,516.1-15.9,1079.9-484.4,1869.9c-536,905.2-730.5,1679.4-603.4,2386
    c75.4,412.9,202.5,726.5,408.9,1044.1c166.8,250.1,571.7,694.8,635.2,694.8C8511.9,5109.8,8670.7,4935.1,8845.3,4724.7z" />
            <path class="st0" d="M5184.9,4665.1l377.2-444.6l-198.5-222.3c-285.8-317.6-397-579.6-420.8-952.8
    c-27.8-504.2,87.3-861.5,492.3-1516.6c258-420.9,412.8-762.3,539.9-1206.9c71.5-254.1,91.3-412.9,91.3-849.6
    c0-619.3-71.5-913.1-325.6-1349.8c-150.9-254.1-639.2-794-718.6-794c-23.8,0-206.4,210.4-401,472.5l-357.3,468.5l178.7,174.7
    c99.3,95.3,226.3,274,285.9,393.1c99.2,198.5,107.2,258.1,103.2,674.9c0,555.8-71.5,770.2-492.3,1468.9
    c-488.3,821.8-663,1409.4-623.3,2124C3751.7,3780,3989.9,4308,4490.2,4840c138.9,146.9,270,270,289.8,270
    C4795.9,5109.8,4982.4,4907.3,5184.9,4665.1z" />
            <path class="st0" d="M1441.1,4704.8c186.6-222.3,353.3-428.8,369.2-456.6c23.8-39.7-19.9-115.1-139-242.2
    c-238.2-258.1-337.5-428.8-412.9-726.5c-123.1-484.3,7.9-1000.5,440.7-1707.1c484.4-794,667-1365.7,671-2100.2
    c0-532-67.5-833.7-285.9-1250.6c-142.9-274-682.9-893.3-778.2-893.3c-35.7,0-595.5,702.7-706.7,893.3
    c-15.9,27.8,43.7,123,154.8,238.2c337.5,353.4,464.5,813.9,373.2,1349.8c-63.5,365.3-186.6,655.1-512.1,1202.9
    C158.8,1782.9,0,2283.1,0,2946.1c0,591.5,146.9,1056,496.3,1556.3c150.9,222.3,516.1,607.4,571.7,607.4
    C1087.8,5109.8,1258.5,4927.2,1441.1,4704.8z" />
          </g>
        </g>
      </svg>
    </p>

    <p class="console"></p>
  </footer>

  <script type="text/javascript">

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
        // checkboxes[i].addEventListener("change", function () {
        //   let checked = false;
        //   for (let j = 0; j < checkboxes.length; j++) {
        //     if (checkboxes[j].checked) {
        //       checked = true;
        //       break;
        //     }
        //   }
        //   document.body.classList.toggle("orange", checked);
        // });
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

    const xhr = new XMLHttpRequest();

    function checkServerAvailability() {
      xhr.open("GET", window.location.hostname);
      xhr.onload = () => {
        if (xhr.status === 200) {
          messageElements.textContent = "Connected";
        } else {
          messageElements.textContent = "Disconected";
        }
      };
      xhr.send();
    }

    setInterval(checkServerAvailability, 300000); // co 5 minut
  </script>
</body>

</html>

)rawliteral";

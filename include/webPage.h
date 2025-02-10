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
    href="//unpkg.com/tailwindcss@^2/dist/tailwind.min.css"
    rel="stylesheet"
  />
  <link href="//cdn.tailwindcss.com/" rel="stylesheet" />
  <link
    rel="stylesheet"
    href="//cdn.jsdelivr.net/npm/swiper@11/swiper-bundle.min.css"
  />
  <script src="//cdn.jsdelivr.net/npm/swiper@11/swiper-bundle.min.js"></script>
  <link
    href="//fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700"
    rel="stylesheet"
    type="text/css"
  />
  <script
    type="text/javascript"
    src="//unpkg.co/gsap@3/dist/gsap.min.js"
  ></script>
  <script
    type="text/javascript"
    src="//unpkg.com/gsap@3/dist/Draggable.min.js"
  ></script>
  <script
    src="//code.jquery.com/jquery-3.6.0.min.js"
    integrity="sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4="
    crossorigin="anonymous"
  ></script>
  <script src="//cdnjs.cloudflare.com/ajax/libs/qrcodejs/1.0.0/qrcode.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/dayjs@1/dayjs.min.js"></script>
  <script src="//cdn.jsdelivr.net/npm/dayjs@1/locale/pl.js"></script>
  <script>
    dayjs.locale("pl");
  </script>
  <script
    type="module"
    src="//unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.esm.js"
  ></script>
  <script
    nomodule
    src="//unpkg.com/ionicons@5.5.2/dist/ionicons/ionicons.js"
  ></script>
  <script src="//cdn.tailwindcss.com"></script>
  <link
    href="//fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700"
    rel="stylesheet"
    type="text/css"
  />
  <link
    href="//unpkg.com/tailwindcss@^2/dist/tailwind.min.css"
    rel="stylesheet"
  />
  <link href="//cdn.tailwindcss.com/" rel="stylesheet" />
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

  <!-- INCLUDE STYLES  -->

  <link href="css/style.css" rel="stylesheet" />

  <!-- END INCLUDE STYLES -->
</head>

<html>
  <body class="">
    <div class="imageBack_container">
      <?xml version="1.0" encoding="UTF-8"?>
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
        />
        <path
          class="cls-1"
          d="m47.97,8.03l4.62-4.62v9.37c-1.99.73-4,1.64-4,3.55,0,2.13,2.48,3.01,4.66,3.79,1.41.5,3.34,1.19,3.34,1.91s-1.93,1.4-3.33,1.9c-2.19.78-4.67,1.66-4.67,3.79s2.48,3.01,4.67,3.79c1.41.5,3.33,1.19,3.33,1.9s-1.98,1.42-3.17,1.84c-.4.14-.67.52-.67.94v6.72c0,.55.45,1,1,1s1-.45,1-1v-6.01c1.85-.68,3.83-1.59,3.83-3.49,0-2.13-2.48-3.01-4.67-3.79-1.41-.5-3.33-1.19-3.33-1.9s1.93-1.4,3.33-1.9c2.19-.78,4.67-1.66,4.67-3.79s-2.48-3.01-4.66-3.79c-1.41-.5-3.34-1.19-3.34-1.91s1.93-1.41,3.33-1.91c.4-.14.67-.52.67-.94V3.41l4.63,4.63c.2.2.45.29.71.29s.51-.1.71-.29c.39-.39.39-1.02,0-1.41L54.3.29c-.09-.09-.2-.17-.33-.22-.24-.1-.52-.1-.76,0-.12.05-.23.12-.33.22l-6.33,6.32c-.39.39-.39,1.02,0,1.41s1.02.39,1.41,0Z"
        />
        <g>
          <path
            class="cls-1"
            d="m50.43,62.22c-.48-.28-1.09-.11-1.37.37-.28.48-.11,1.09.37,1.37l.31.18h4l-3.31-1.91Z"
          />
          <path
            class="cls-1"
            d="m.5,35.7l43.01,24.83c.15.09.33.13.5.13s.35-.04.5-.13l8.1-4.68,14.33,8.27h3.93c-.17-.13-.36-.25-.56-.36l-15.7-9.07,6.6-3.81,16.7,9.64c1.35.78,3.07,1.17,4.8,1.17s3.45-.39,4.8-1.17c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37-2.06,1.19-5.54,1.19-7.6,0l-15.7-9.07,24.31-14.03c.31-.18.5-.51.5-.87s-.19-.69-.5-.87l-22.3-12.88c-.48-.28-1.09-.11-1.37.37-.28.48-.11,1.09.37,1.37l20.8,12.01-41.01,23.68L3,34.83l20-11.55c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37L.5,33.97C.19,34.15,0,34.48,0,34.83s.19.69.5.87Z"
          />
        </g>
        <g>
          <path
            class="cls-1"
            d="m17.7,55.57l14.83,8.56h4l-17.83-10.3c-.81-.47-1.28-1.06-1.28-1.62s.47-1.15,1.28-1.62c.48-.28.64-.89.37-1.37-.28-.48-.89-.64-1.37-.37-1.47.85-2.28,2.04-2.28,3.35s.81,2.5,2.28,3.35Z"
          />
          <path
            class="cls-1"
            d="m1.5,60.53c2.06-1.19,5.54-1.19,7.6,0l6.23,3.6h4l-9.23-5.33c-2.69-1.55-6.91-1.55-9.6,0-.48.28-.64.89-.37,1.37.28.48.89.64,1.37.37Z"
          />
        </g>
      </svg>
    </div>
    <div class="containerouter">
      <div class="container">
        <h3 class="noselect">Przedpokój</h3>
        <div class="checkbox-container purple noselect">
          <ion-icon
            class="handIcon disabled"
            name="hand-right-outline"
          ></ion-icon>
          <input type="checkbox" class="switch noselect" id="pin_0" />
          <label for="pin_0"></label>
          <div class="active-circle"></div>
        </div>
      </div>

      <div class="container">
        <h3 class="noselect">Sypialnia</h3>
        <div class="checkbox-container purple noselect">
          <ion-icon
            class="handIcon disabled"
            name="hand-right-outline"
          ></ion-icon>
          <input type="checkbox" class="switch noselect" id="pin_1" />
          <label for="pin_1"></label>
          <div class="active-circle"></div>
        </div>
      </div>

      <div class="container">
        <h3 class="noselect">Kuchnia</h3>
        <div class="checkbox-container purple noselect">
          <ion-icon
            class="handIcon disabled"
            name="hand-right-outline"
          ></ion-icon>
          <input type="checkbox" class="switch noselect" id="pin_2" />
          <label for="pin_2"></label>
          <div class="active-circle"></div>
        </div>
      </div>

      <div class="container">
        <h3 class="noselect">Łazienka</h3>
        <div class="checkbox-container purple noselect">
          <ion-icon
            class="handIcon disabled"
            name="hand-right-outline"
          ></ion-icon>
          <input type="checkbox" class="switch noselect" id="pin_3" />
          <label for="pin_3"></label>
          <div class="active-circle"></div>
        </div>
      </div>

      <div class="container disabled">
        <h3 class="noselect">Klatka schodowa</h3>
        <div class="checkbox-container purple noselect">
          <ion-icon
            class="handIcon disabled"
            name="hand-right-outline"
          ></ion-icon>
          <input type="checkbox" class="switch noselect" disabled id="pin_4" />
          <label for="pin_4"></label>
          <div class="active-circle"></div>
        </div>
      </div>
      <div class="container disabled">
        <h3 class="noselect">Gościnny</h3>
        <div class="checkbox-container purple noselect">
          <ion-icon
            class="handIcon disabled"
            name="hand-right-outline"
          ></ion-icon>
          <input type="checkbox" class="switch noselect" id="pin_0" disabled />

          <label for="pin_0"></label>
          <div class="active-circle"></div>
        </div>
      </div>
    </div>

    <div class="ifdata">
      <div class="container-modal unvisible">
        <div class="modal-back"></div>

        <div
          class="weather_widget weather_section flex flex-col items-center justify-center min-h-screen text-gray-700 p-10"
        >
          <div
            class="close noselect bg-white p-10 rounded-xl ring-8 ring-white ring-opacity-40"
          >
            +
          </div>
          <div
            class="w-full max-w-screen-sm owHidden bg-white p-10 rounded-xl ring-8 ring-white ring-opacity-40"
          >
            <div class="flex justify-between currentHour">
              <!-- Componnent currentHour -->
            </div>

            <hr />

            <div class="nextHours swiper">
              <!-- Component nextHours -->
            </div>
            <hr />

            <div class="scrollHandle nextDays">
              <!-- Component nextDays -->
            </div>
          </div>
        </div>
      </div>
    </div>

    <footer>
      <a href="config" class="noselect">
        <ion-icon name="cog-outline" size="large"></ion-icon>
      </a>

      <a href="#" class="noselect usegaz">
       
        <ion-icon name="scale-outline"></ion-icon>
      </a>

      <a href="#" class="noselect a_fire">
        <ion-icon name="bonfire-outline"></ion-icon>
      </a>

      <a href="#" class="noselect a_weather"></a>
      <a href="#" class="noselect a_qrcode">
        <ion-icon name="qr-code-outline"></ion-icon>
      </a>

      <p>
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

      <p class="console noselect"></p>
    </footer>

    <div class="qr-code ring-8 ring-white ring-opacity-40" ></div>
  </body>

  <!-- ARDUINO PART  INCLUDE JS -->

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


</script>
  <script type="text/javascript">let owapikey = '5d652f0bff80d46c6a9ce422c48e4296';
let lat = '49.88087';
let lon = '19.56303';
let data = {};
let srcOfImage = "";


function random(min, max) {
  return Math.floor(Math.random() * (max - min + 1) + min);
}

function convertTimestamp(timestamp, type) {
  let date = new Date(timestamp * 1000);
  let hours = date.getHours();
  let minutes = date.getMinutes();
  let seconds = date.getSeconds();
  let day = date.getDate();
  let month = date.getMonth() + 1;
  let year = date.getFullYear();

  if (type == "date") {
    return `${day}.${month}.${year}`;
  }
  else if (type == "time") {
    return `${hours}:${minutes}:${seconds}`;
  }
  else if (type == "day") {
    return day;
  }
  else if (type == "month") {
    return month;
  }
  else if (type == "year") {
    return year;
  }
  else if (type == "hours") {
    return hours;
  }
  else if (type == "minutes") {
    return minutes;
  }
  else if (type == "seconds") {
    return seconds;
  } else return false;
}

// get forecast
function getForecast() {
  fetch(
    // new API 
    "//api.openweathermap.org/data/3.0/onecall?lat=" + lat + "&lon=" + lon + "&appid=" + owapikey + "&units=metric"
  )
    .then((response) => response.json())
    .then((data_) => {

      // console.log(data_.current.temp);
      if (data_.current.temp == '-0') { data_.current.temp = 0 }
      //data = data_;
      srcOfImage = "//openweathermap.org/img/wn/" + data_.current.weather[0].icon + "@2x.png";
      document.querySelector('.ifdata').classList.toggle('active');
      $("footer .a_weather").append(`<img  src="//${srcOfImage}" alt=""/>`)
      $("footer .a_weather").show();
      $('.container-modal').hide();
      $('.weather_widget .close, footer .a_weather').bind('click', function () {
        $('.container-modal').toggle(300);
      });

      currentHour(data_);
      nextHours(data_['hourly']).then(() => {

        const swiper = new Swiper('.swiper', {
          grabCursor: true,
          watchSlidesProgress: true,
          slidesPerView: 3,
          autoplay: {
            delay: 4500,
            disableOnInteraction: true
          }

        });

      }).then(
        nextDays(data_['daily'])
      );
    });
}

let x = 0;
let list;

// Component nextHours   
let nextHours = async (data) => {
  let html = `<div class="swiper-wrapper">`;
  if (data) {


    data.forEach(nexthour => {

      let el = `<div class="swiper-slide flex time_items flex-col items-center noSelect" style="margin-right:30px; padding-bottom: 20px;">
                <span class="font-semibold mt-1 text-sm lcd noSelect time_dis">${convertTimestamp(nexthour.dt, "hours")}:${convertTimestamp(nexthour.dt, "minutes")} </span>
                <span class="text-xs font-semibold text-gray-400 lcd noSelect AMPM">PM</span>       
                <img alt="" src="////openweathermap.org/img/wn/${nexthour.weather[0].icon}@2x.png" width="120px" class="noSelect" style="pointer-events: none;"/>	 
                <span class="font-semibold text-lg lcd noSelect">${nexthour.temp.toFixed(1)}°C</span>      <span class="text-sm lcd noSelect">${nexthour.feels_like}</span></div>`;
      html = html + el;
    });
    $('.nextHours').html("").append(html + `</div>`);
    return true;
  }
}
// Component currentHour   
let currentHour = async (data) => {
  let html = "";
  if (data) {

    console.log(data);

    html = `<div class="flex flex-col">
            <span class="text-3xl font-bold lcd">${data.current.temp}<span class="text-3xl oc">°C</span></span>
            <span class="font-light mt-1 text-gray-500 odczuwalna">Odczuwalna : ${data.current.feels_like.toFixed(1)}</span>
            <!-- <span class="font-semibold mt-1 text-gray-500">${data.name}</span> -->
            </div>
            <div class="flex flex-col">
            <span class="text-3xl font-bold lcd">&nbsp${data.current.humidity.toFixed(0)}<span class="text-3xl"><span
              class="upperTop">o</span>/o</span></span>

            <!-- <span class="font-semibold mt-1 text-gray-500">${data.name}</span> -->
            </div>
            <div class="h-24 w-24 ">
            <img  src="//${srcOfImage}" alt="" class="currentWeatherIcon"/>
            </div>`;
    $('.currentHour').html("").append(html);
    return true;
  }
}


let nextDays = async (data) => {

  if (data) {
    let html;

    data.forEach(day => {
      el = `<div class="flex justify-between items-center">
              <span class="font-semibold text-sm w-1/4 firstLetterUp">
              ${dayjs.unix(day.dt).format('dddd')}<br>
              ${dayjs.unix(day.dt).locale('pl').format('D')} ${dayjs.unix(day.dt).format('MMM')}
              </span>
            <div class="flex items-center justify-end w-1/4 pr-10">
                <span class="font-semibold humidityForecast">
                ${day.humidity}%
                </span>
            <IconHumidity></IconHumidity>
            </div>
              <img alt="" src="////openweathermap.org/img/wn/${day.weather[0].icon}@2x.png" width="" />	
              <span class="font-semibold text-lg w-1/4 text-right temp_primary">${day.temp.day}°</span>
              <span class="under">
              <span class="fontTemp w-1/2 text-right temp_secondary"><span class="labelTemp">min
              </span>${day.temp.min}°</span>
              <span class="fontTemp w-1/2 text-right temp_secondary"><span class="labelTemp">max
              </span>${day.temp.max}°</span>
              </span>
            </div>`
      html = html + el;
    })
    $('.nextDays').html("").append(html);
    return true;
  } else { return false }
}
// setInterval(getForecast, 20000) // 33,33 minuty 



</script>
  <script type="text/javascript">
var qrcode_=document.querySelector(".qr-code");
$(".qr-code").hide();





function generateQR(user_input){


var qrcode = new QRCode(qrcode_, {
    text: user_input,
    width: 256, //128
    height: 256,
    colorDark : "#000000",
    colorLight : "#ffffff",
    correctLevel : QRCode.CorrectLevel.H
});



let ipc = document.createElement("p");
ipc.innerText = user_input;
document.querySelector(".qr-code").appendChild(ipc);


let close_qr = document.createElement("a");
close_qr.innerText = "+";
close_qr.classList.add("qr-close", "ring-8", "ring-white", "ring-opacity-40");
document.querySelector(".qr-code").appendChild(close_qr);

$("a.a_qrcode, .qr-code .qr-close").on("click", function(){
  $(".qr-code").toggle(400);
});



}

</script>

  <!-- END ARDUINO PART  INCLUDE JS -->
</html>


)rawliteral";
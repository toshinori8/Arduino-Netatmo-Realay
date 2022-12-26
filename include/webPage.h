#include <Arduino.h>

const char webpage[] PROGMEM = R"rawliteral(

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,minimum-scale=1">
    <meta name="theme-color" content="#474E65">
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="application-name" content="Netatmo Relay">
    <title>Netatmo Relay</title>
    <link href='https://cdnjs.cloudflare.com/ajax/libs/tailwindcss/2.2.19/tailwind.min.css' rel='stylesheet' type='text/css'>
    <link href='https://fonts.googleapis.com/css?family=Open+Sans:400,700,300,600|Open+Sans+Condensed:300,700' rel='stylesheet' type='text/css'>
    <script type="text/javascript" src="https://unpkg.co/gsap@3/dist/gsap.min.js"></script>
    <script type="text/javascript" src="https://unpkg.com/gsap@3/dist/Draggable.min.js"></script>
    <script src="https://code.jquery.com/jquery-3.6.0.min.js" integrity="sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4=" crossorigin="anonymous"></script>

  <link src="http://code.ionicframework.com/ionicons/2.0.1/css/ionicons.min.css" src_type="url" rel="stylesheet" type="text/css"/>



    <link rel="icon" href="data:image/svg+xml,%0A%3Csvg version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' x='0px' y='0px' width='166.9px' height='166.9px' viewBox='0 0 166.9 166.9' style='overflow:visible;enable-background:new 0 0 166.9 166.9;' xml:space='preserve'%3E%3Cstyle type='text/css'%3E .st0%7Bfill:%237C3747;%7D .st1%7Bfill:%23A46A63;%7D .st2%7Bfill:%23FFFFFF;%7D%0A%3C/style%3E%3Cdefs%3E%3C/defs%3E%3Ccircle class='st0' cx='83.4' cy='83.4' r='83.4'/%3E%3Ccircle class='st1' cx='83.4' cy='83.4' r='61.5'/%3E%3Cg%3E%3Cpath class='st2' d='M49.7,72.9l-11.3-2.2c0.9-4,2.7-7.1,5.4-9.2c2.7-2.1,6.5-3.2,11.4-3.2c5.6,0,9.7,1.2,12.2,3.5 c2.5,2.3,3.8,5.2,3.8,8.8c0,2.1-0.5,3.9-1.5,5.6c-1,1.7-2.6,3.1-4.6,4.4c1.7,0.5,2.9,1,3.8,1.6c1.4,1,2.5,2.3,3.3,3.9 c0.8,1.6,1.2,3.5,1.2,5.7c0,2.8-0.7,5.4-2,8c-1.3,2.5-3.2,4.5-5.7,5.9s-5.7,2.1-9.7,2.1c-3.9,0-7-0.5-9.3-1.5 c-2.3-1-4.1-2.5-5.6-4.5c-1.5-2-2.6-4.4-3.4-7.4l12-1.8c0.5,2.7,1.2,4.5,2.2,5.5c1,1,2.2,1.5,3.8,1.5c1.6,0,2.9-0.7,4-2 c1.1-1.3,1.6-3,1.6-5.2c0-2.2-0.5-3.9-1.5-5.1c-1-1.2-2.4-1.8-4.2-1.8c-0.9,0-2.2,0.3-3.9,0.8l0.6-9.5c0.7,0.1,1.2,0.2,1.6,0.2 c1.6,0,2.9-0.6,3.9-1.7c1-1.1,1.6-2.4,1.6-3.9c0-1.5-0.4-2.6-1.2-3.5c-0.8-0.9-1.9-1.3-3.2-1.3c-1.4,0-2.6,0.5-3.4,1.4 S50,70.6,49.7,72.9z'/%3E%3Cpath class='st2' d='M113.3,106.8H77.4c0.4-3.9,1.7-7.6,3.7-11.1c2.1-3.5,6-7.5,11.7-12.3c3.5-2.9,5.8-5.1,6.7-6.6 c1-1.5,1.5-2.9,1.5-4.3c0-1.5-0.5-2.7-1.5-3.7c-1-1-2.2-1.5-3.7-1.5c-1.5,0-2.8,0.5-3.7,1.6c-1,1.1-1.6,2.9-2,5.6l-12-1.1 c0.5-3.7,1.3-6.6,2.6-8.7c1.3-2.1,3-3.7,5.3-4.8c2.3-1.1,5.4-1.7,9.5-1.7c4.2,0,7.5,0.5,9.8,1.6c2.3,1.1,4.2,2.7,5.5,4.9 c1.3,2.2,2,4.7,2,7.4c0,2.9-0.8,5.7-2.3,8.3c-1.5,2.6-4.3,5.6-8.4,8.7c-2.4,1.8-4,3.1-4.8,3.9s-1.8,1.7-2.9,2.9h18.7V106.8z'/%3E%3Cpath class='st2' d='M136.3,67.6c0,2.6-0.8,4.8-2.5,6.6c-1.6,1.8-3.6,2.7-6,2.7c-2.3,0-4.3-0.9-5.9-2.7c-1.6-1.8-2.5-4-2.5-6.6 c0-2.6,0.8-4.8,2.5-6.6c1.6-1.8,3.6-2.7,5.9-2.7c2.3,0,4.3,0.9,6,2.7C135.5,62.8,136.3,65,136.3,67.6z M130.8,67.6 c0-0.9-0.3-1.7-0.9-2.3c-0.6-0.7-1.3-1-2.1-1c-0.8,0-1.5,0.3-2.1,1s-0.9,1.4-0.9,2.3c0,0.9,0.3,1.7,0.9,2.3c0.6,0.6,1.3,1,2.1,1 c0.8,0,1.5-0.3,2.1-1C130.5,69.3,130.8,68.5,130.8,67.6z'/%3E%3C/g%3E%3C/svg%3E%0A
" type="image/svg+xml" />

        

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

        

        body {
            font-family: OpenSans, sans-serif;
            height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            background: var(--l3);
            color: var(--text);
            display: grid;
            place-items: center;
            font: 1em/1.5 Nunito, sans-serif;
            height: 100vh;
        }

       

        

        

        .box {
            background-color: var(--l4);
            border-radius: 0.75em;
            box-shadow:
                0 0.1em 0.1em var(--l5) inset,
                0 -0.1em 0.1em var(--l2) inset;
            display: flex;
            justify-content: space-between;
            padding: 0.75em 1.75em;
            text-align: center;
        }

        
        /* Animations */
        @keyframes digitA {
            from {
                opacity: 1;
                transform: translateY(0);
            }

            to {
                opacity: 0;
                transform: translateY(-25%%);
            }
        }

        @keyframes digitB {
            from {
                opacity: 0;
                transform: translateY(25%%);
            }

            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .input_button {

            border-radius: 18px;
            background-color: var(--l3);
            border: 1px solid var(--l6);
            padding: 10px;
        }

        /** CSS INPUT SWITCH **/
        .on-off-toggle {
            width: 56px;
            height: 24px;
            position: relative;
            display: inline-block;
            float: right;
        }

        .on-off-toggle__slider {
            width: 56px;
            height: 24px;
            display: block;
            border-radius: 34px;
            background-color: #d8d8d8;
            transition: background-color 0.4s
        }

        .on-off-toggle__slider:before {
            content: '';
            display: block;
            background-color: #fff;
            box-shadow: 0 0 0 1px var(--text);
            bottom: 3px;
            height: 18px;
            left: 3px;
            position: absolute;
            transition: .4s;
            width: 18px;
            z-index: 5;
            border-radius: 100%%;
        }

        .on-off-toggle__slider:after {
            display: block;
            line-height: 24px;
            text-transform: uppercase;
            font-size: 12px;
            font-weight: bold;
            content: 'off';
            color: #484848;
            padding-left: 26px;
            transition: all 0.4s;
        }

        .on-off-toggle__input {
            position: absolute;
            opacity: 0;
        }

        .on-off-toggle__input:checked+.on-off-toggle__slider {
            background-color: var(--green)
        }

        .on-off-toggle__input:checked+.on-off-toggle__slider:before {
            transform: translateX(32px);
        }

        .on-off-toggle__input:checked+.on-off-toggle__slider:after {
            content: 'on';
            color: #FFFFFF;
            padding-left: 8px;
        }

        .input_button p {
            float: left;
            font-size: 0.7em;
            opacity: 0.8;
            margin-right: 20px;
            color: var(--text);
        }

        /*** END CSS NEEDED FOR SWITCH **/




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

    </style>

<style>
      .neumorphism-button {
        display: inline-block;
        padding: 10px 20px;
        border-radius: 5px;
        background-color: white;
        box-shadow: 2px 2px 10px rgba(0, 0, 0, 0.1), -2px -2px 10px rgba(255, 255, 255, 0.5);
      }
  
      .neumorphism-button:active {
        box-shadow: inset 2px 2px 10px rgba(0, 0, 0, 0.1), inset -2px -2px 10px rgba(255, 255, 255, 0.5);
      }
    </style>

</head>

<body class="">
    <div class="temp">


        

    <button class="neumorphism-button">Room 1</button>
    <button class="neumorphism-button">Room 2</button>
    <button class="neumorphism-button">Room 3</button>
    <button class="neumorphism-button">Room 4</button>
    <button class="neumorphism-button">Room 5</button>
    <button class="neumorphism-button">Room 6</button>
 

        <div class="box">
            <div class="temp__outdoors-col">
                <small class="temp__heading">Current</small>
                <br>
                <span class="temp__c-value">0°</span>
            </div>
            <div class="temp__outdoors-col">
                <small class="temp__heading">Outside</small>
                <br>
                <span class="temp__o-value">0°</span>
            </div>
            <div class="temp__outdoors-col">
                <small class="temp__heading">Humidity</small>
                <br>
                <span class="temp__h-value">0%%</span>
            </div>
        </div>

        <div class="input_button">
            <div class="on-off-toggle">

                <input class="on-off-toggle__input" checked type="checkbox" id="bopis" />
                <label for="bopis" class="on-off-toggle__slider"></label>
            </div>
            <p class="labell">wiatrak</p>


        </div>
        <div class="box">

          
          <button class="button button-icon button-clear md-settings">Ustawienia</button>

          
          <ion-icon name="md-settings"  class="settings" ios="ios-settings" md="md-settings"></ion-icon>


          <script type="javascript">
          
          
          </script>

        
        </div>

<a href='config'>Konfiguracja</a>

    </div>


    
    <script type="text/javascript">
        var startAnim = gsap.to(".ring", {
            rotation: "+=360",
            ease: "power1.in",
            paused: true,
            duration: 3,
            onComplete: function() {

                startAnim.pause();
                loopAnim.play(0);

            }
        });

        var loopAnim = gsap.to(".ring", {
            rotation: "+=360",
            ease: "none",

            repeat: -1,
            onComplete: function() {
                stateAnim = "play";
            },
            paused: true
        });



        var stopAnim = gsap.to(".ring", {
            rotation: 180,
            duration: 0.6,
            paused: true,
            onComplete: function() {

                loopAnim.pause();
                startAnim.pause();

            }
        });


        // $("#start").on('click', function() {


        //     if (loopAnim.paused() & startAnim.paused()) {

        //         startAnim.play(0);


        //     }

        // });

        $("#stop").on('click', function() {
            if (!loopAnim.paused() & startAnim.paused() || !startAnim.paused()) {
                stopAnim.play(0);
            }
        });







        document.addEventListener("DOMContentLoaded", function() {



            
            $.ajax({
                type: "GET",
                url: "https://api.openweathermap.org/data/2.5/weather?lat=49.880870&lon=19.563030&appid=f055d509de51700a688e61d5f8e3da76&units=metric",
                dataType: "json",
                success: function(json) {

                    console.log('remote data fetched :'+ json.main);
                   // thermostat.outdoorsAdjust(json.main.temp, json.main.humidity);
                   
                }
            });

             function fetchAjax() {
                    $.ajax({
                        type: "GET",
                        url: "/params",
                        dataType: "json",
                        success: function(datax) {
                            console.log(datax);
                            //thermostat.tempAdjust(datax.setTemp);
                            $(".temp__c-value").html(datax.temp + "°")

                            if (datax.fan == "checked") {
                                $(".on-off-toggle__input").prop('checked', true);
                                var lAnimPaused = loopAnim.paused();

                                if (loopAnim.paused() & startAnim.paused()) {

                                    startAnim.play(0);


                                }

                            } else {
                                $(".on-off-toggle__input").prop('checked', false);
                                if (!loopAnim.paused() & startAnim.paused() || !startAnim.paused()) {
                                    stopAnim.play(0);
                                }

                            }

                        }

                    });

                };

                    
            setInterval(fetchAjax, 1200);
        //     $.ajax({
        //                     "url": "/setTemp?setTemp=" + actualTemp.toFixed(0)
        //                 });

        // let actualTemp = 0;


        // });

          

    </script>

</body>

</html>


)rawliteral";

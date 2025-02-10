let owapikey = '5d652f0bff80d46c6a9ce422c48e4296';
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




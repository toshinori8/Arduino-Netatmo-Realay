const fs = require("fs");
const express = require("express");
const path = require("path");
const { loadTokens, refreshaccess_token } = require("./tokenize.js");
const config = JSON.parse(fs.readFileSync(path.join(__dirname, "config.json")));
const qs = require("qs");
const axios = require("axios");

const {
  roomExists,
  setRoomTemperature,
  setHomeMode,
  checkTemperatureSet,
} = require("./setData");
const {
  filterHomestatusData,
  filterHomesdata,
  combineRoomData,
} = require("./getData");

let data = qs.stringify({});

const app = express();
const PORT = 3001;

app.use(express.json());

app.get("/getdata", async (req, res) => {
  try {
    const tokens = await loadTokens();
    const homeId = config.homeId;
    const accessToken = tokens.access_token;

    const xhrconfig = {
      method: "get",
      maxBodyLength: Infinity,
      url: `${config.apiUrl}/api/homestatus?access_token=${encodeURIComponent(
        accessToken
      )}&home_id=${encodeURIComponent(homeId)}`,
      headers: {},
    };

    const xhrconfig2 = {
      method: "get",
      maxBodyLength: Infinity,
      url: `${config.apiUrl}/api/homesdata?access_token=${encodeURIComponent(
        accessToken
      )}`,
      headers: {},
    };

    let outDATA = {};
    let response1 = {};
    let response2 = {};
    let chk = 0;

    axios
      .request(xhrconfig)
      .then((response) => {
        response1 = response.data;
        return axios.request(xhrconfig2);
      })
      .then((response) => {
        response2 = response.data;
        let filteredResponse1 = filterHomestatusData(response1);
        let filteredResponse2 = filterHomesdata(response2);
        outDATA = {
          rooms: combineRoomData(filteredResponse1, filteredResponse2),
          // meta: {
          //   homestatus: response1,
          //   homesdata: response2,
          // },
        };
        res.json(outDATA);
      })
      .catch((error) => {
        console.log("Error making GET request:", error.message);
        if (
          error.response &&
          error.response.data &&
          error.response.data.error &&
          error.response.data.error.code === 3
        ) {
          console.log("Error : ", error.response.data.error.message);
          res.status(400).json({ error: error.response.data.error.message });
        
          if(error.response.data.error.message === "Access token expired") {
            console.log("Refreshing token");
            refreshaccess_token();
          }

        } else  {
          res.status(500).json({ error: error.message });
        }

      });
  } catch (error) {
    console.error("Error retrieving data:", error);
    res.status(500).send("Error retrieving data");
  }
});


app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});

// set function  set temperature and mode
app.get("/setRoomTemperature", async (req, res) => {

console.log("setRoomTemperature")
  try {
    const tokens = await loadTokens();
    let { access_token, refresh_token } = tokens;

    const { mode, temperature, room_id } = req.query;
    setRoomTemperature(access_token, refresh_token, room_id, temperature);

    res.json({ message: "Temperature set successfully" });
  } catch (error) {
    console.error("Error setting data:", error);
    res.status(500).send("Error setting data");
  }
});




app.get("/setdata", async (req, res) => {
  try {
    const tokens = await loadTokens();
    let { access_token, refresh_token } = tokens; 
    const { mode, temperature, room_id } = req.query;
    let response;
    if (mode === "away") {    
      response = await setHomeMode(access_token, refresh_token, "away");
    } else {
      if (!room_id) { 
        return res.status(400).json({ error: "Missing room_id parameter" });
      }
      const roomExistsResult = await roomExists(access_token, refresh_token, room_id);
      if (!roomExistsResult) {
        return res.status(400).json({ error: "Room does not exist" });
      }
      if (!temperature) {
        return res.status(400).json({ error: "Missing temperature parameter" });
      }
      response = await setRoomTemperature(access_token, refresh_token, room_id, temperature);
      await new Promise((resolve) => setTimeout(resolve, 2000)); // Poczekaj 5 sekund
      // const temperatureSet = await checkTemperatureSet(access_token, refresh_token, room_id, temperature);

      return res.json({ response, temperature_set: temperatureSet });
    }
  } catch (error) {
    console.error("Error setting data:", error);
    res.status(500).send("Error setting data");
  }
});



// app.get('/setdata', async (req, res) => {
//   try {
//     const tokens = await loadTokens();
//     let { access_token, refresh_token } = tokens;

//     const { mode, temperature, room_id } = req.query;
//     let response;
//     if (mode === 'away') {
//       response = await setHomeMode(access_token, refresh_token, 'away');
//     } else {
//       if (!room_id) {
//         return res.status(400).json({ error: 'Missing room_id parameter' });
//       }

//       const roomExistsResult = await roomExists(access_token, refresh_token, room_id);
//       if (!roomExistsResult) {
//         return res.status(400).json({ error: 'Room does not exist' });
//       }

//       if (!temperature) {
//         return res.status(400).json({ error: 'Missing temperature parameter' });
//       }

//       response = await setRoomTemperature(access_token, refresh_token, room_id, temperature);

//       await new Promise(resolve => setTimeout(resolve, 5000)); // Poczekaj 5 sekund
//       const temperatureSet = await checkTemperatureSet(access_token, refresh_token, room_id, temperature);
//       return res.json({ response, temperature_set: temperatureSet });
//     }
//   } catch (error) {
//     console.error('Error setting data:', error);
//     res.status(500).send('Error setting data');
//   }
// });
// async function refreshaccess_token() {
  // try {
  //   const payload = new URLSearchParams({
  //     grant_type: "refresh_token",
  //     refresh_token: config.refresh_token,
  //     client_id: config.clientId,
  //     client_secret: config.clientSecret,
  //   }).toString();
  //   console.log("Payload : ", payload);
  //   const response = await axios.post(
  //     "https://api.netatmo.com/oauth2/token",
  //     payload,
  //     {
  //       headers: {
  //         "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8",
  //       },
  //     }
  //   );
  //   //config.access_token = response.data.access_token;
  //   // config.refresh_token = response.data.refresh_token;  // Może się zmienić
  //   // Zapisanie nowych tokenów do pliku
  //   //saveTokensToFile(config.access_token, config.refresh_token);
  //   console.log("Tokens refreshed successfully.");
  // } catch (error) {
  //   // console.error('Error refreshing token:', error);
  //   //throw error;
  // }
// }

// Funkcja do odczytu tokenów z config.json
// function readTokensFromFile() {
//   try {
//     const data = fs.readFileSync("config.json", "utf8");
//     const tokens = JSON.parse(data);
//     return tokens;
//   } catch (err) {
//     console.error("Error reading tokens from file:", err);
//     return null; // Jeśli plik nie istnieje, zwróci null
//   }
// }

// Funkcja do zapisania tokenów do config.json
// function saveTokensToFile(access_token, refresh_token) {
//   const tokens = { access_token, refresh_token };
//   fs.writeFileSync("config.json", JSON.stringify(tokens, null, 2), "utf8");
// }

// app.listen(PORT, () => {
//   console.log(`Server is running on http://localhost:${PORT}`);
// });



// Endpoint do pobierania danych z API Netatmo

// let url = `${config.apiUrl}/api/homesdata?access_token=${encodeURIComponent(
//   config.access_token
// )}`;
// console.log("urL", url);

// Funkcja do odświeżania Access Token za pomocą Refresh Token
// async function refreshaccess_token() {
//   try {
//     const response = await axios.post("https://api.netatmo.com/oauth2/token", {
//       grant_type: "refresh_token",
//       refresh_token: config.refresh_token,
//       client_id: config.clientId,
//       client_secret: config.clientSecret,
//     });
//     access_token = response.data.access_token;
//     refresh_token = response.data.refresh_token; // Może się zmienić
//     console.log("Tokens refreshed successfully.");
//   } catch (error) {
//     console.error("Error refreshing token:", error);
//     throw error;
//   }
// }

// // Wywołanie funkcji do pobrania danych
// getData()
//   .then(data => console.log('Data:', data))
//   .catch(error => console.error('Failed to fetch data:', error));

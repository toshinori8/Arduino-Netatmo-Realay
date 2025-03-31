const axios = require('axios');
const { refresh_token, saveTokens } = require('./tokenize');
const config = require('./config');

async function roomExists(access_token, refresh_token, roomId) {
  let url = `${config.apiUrl}/api/homesdata?access_token=${encodeURIComponent(access_token)}`;
  let response = await makeGetRequest(url);

  if (response.error && (response.error.code === 3 || response.error.code === 2)) {
    access_token = await refresh_token(refresh_token);
    saveTokens(access_token, refresh_token);
    response = await makeGetRequest(url);
  }

  if (response.body.homes[0].rooms) {
    return response.body.homes[0].rooms.some(room => room.id === roomId);
  }

  return false;
}
///

// POST
//  /setroomthermpoint Control the heating of a room
// This endpoint permits to control the heating of a specific room. A room can be set in 3 differents modes:

// "manual" mode in which setpoints can be applied for a specific duration (use endtime)

// "max" mode which will put the setpoint at max (30°) for a given duration (default is xx and can be changed by the user in its settings)

// "home" mode which basically makes the room following the Home mode.


//  Scope to use: 
// write_thermostat

// Name
// Description
// home_id *required
// string
// (query)
// Home ID
// room_id *required
// string
// (query)
// Room ID
// mode *required
// string
// (query)
// The mode you are applying to this room {manual, max, home}
// temp
// number
// (query)
// Manual temperature to apply. Required if mode is manual.
// endtime
// string
// (query)
// End time of the schedule mode set (Local Unix Time in seconds).

async function setRoomTemperature(access_token, refresh_token, roomId, temperature) {
  const url = `${config.apiUrl}/api/setroomthermpoint`;
  const data = {
    home_id: config.homeId,
    room_id: roomId,
    mode: 'manual',
    temp: temperature
  };

  console.log(url);
  console.log(data);

  let response = await makePostRequest(url, access_token, data);
  console.log('Response from setRoomTemperature:', response);
  if (response.error && (response.error.code === 3 || response.error.code === 2)) {
    access_token = await refresh_token(refresh_token);
    saveTokens(access_token, refresh_token);
    response = await makePostRequest(url, access_token, data);
  }
  return response;
}

async function setHomeMode(access_token, refresh_token, mode) {
  const url = `${config.apiUrl}/api/setthermmode`;
  const data = {
    home_id: config.homeId,
    mode: mode
  };

  let response = await makePostRequest(url, access_token, data);

  if (response.error && (response.error.code === 3 || response.error.code === 2)) {
    access_token = await refresh_token(refresh_token);
    saveTokens(access_token, refresh_token);
    response = await makePostRequest(url, access_token, data);
  }

  return response;
}

async function checkTemperatureSet(access_token, refresh_token, roomId, temperature) {
  let url = `${config.apiUrl}/api/homestatus?access_token=${encodeURIComponent(access_token)}&home_id=${encodeURIComponent(config.homeId)}`;
  let response = await makeGetRequest(url);

  if (response.error && (response.error.code === 3 || response.error.code === 2)) {
    access_token = await refresh_token(refresh_token);
    saveTokens(access_token, refresh_token);
    response = await makeGetRequest(url);
  }

  if (response.body.home.rooms) {
    return response.body.home.rooms.some(room => room.id === roomId && room.therm_setpoint_temperature === temperature);
  }

  return false;
}

async function makePostRequest(url, access_token, data) {
  try {
    const response = await axios.post(url, new URLSearchParams(data), {
      headers: { Authorization: `Bearer ${access_token}` }
    });
    return response.data;
  } catch (error) {
    console.error('Error making POST request:', error.response.data);
    throw new Error('Error making POST request');
  }
}

async function makeGetRequest(url) {
  try {
    const response = await axios.get(url);
    return response.data;
  } catch (error) {
    console.error('Error making GET request:', error.response.data);
    throw new Error('Error making GET request');
  }
}

module.exports = { roomExists, setRoomTemperature, setHomeMode, checkTemperatureSet, makePostRequest, makeGetRequest };
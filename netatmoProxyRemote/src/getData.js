
const fs = require('fs');
const path = require('path');
const axios = require('axios');


const config = JSON.parse(fs.readFileSync(path.join(__dirname, 'config.json')));


function filterHomestatusData(data) {
  if (!data.body.home.rooms) {
    return [];
  }

  return data.body.home.rooms.map(room => ({
    id: room.id || null,
    reachable: room.reachable || null,
    anticipating: room.anticipating || null,
    heating_power_request: room.heating_power_request || null,
    open_window: room.open_window || null,
    therm_measured_temperature: room.therm_measured_temperature || null,
    therm_setpoint_temperature: room.therm_setpoint_temperature || null,
    therm_setpoint_mode: room.therm_setpoint_mode || null
  }));
}

function filterHomesdata(data) {
  if (!data.body.homes[0].rooms) {
    return [];
  }

  return data.body.homes[0].rooms.map(room => ({
    id: room.id || null,
    name: room.name || null,
    type: room.type || null
  }));
}

function combineRoomData(homestatusRooms, homesdataRooms) {
  return homestatusRooms.map(statusRoom => {
    const dataRoom = homesdataRooms.find(room => room.id === statusRoom.id);
    return { ...statusRoom, ...dataRoom };
  });
}

module.exports = { filterHomestatusData, filterHomesdata, combineRoomData };
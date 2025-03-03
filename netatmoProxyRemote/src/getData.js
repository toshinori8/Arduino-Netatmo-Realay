
const fs = require('fs');
const path = require('path');
const axios = require('axios');


const config = JSON.parse(fs.readFileSync(path.join(__dirname, 'config.json')));



//The enrich_rooms_with_battery_state function would need to be translated to javascript.
function extractBatteryState(data) {
  // Create a dictionary to map module IDs to battery states
  const modules = {};
  data.meta.homestatus.body.home.modules.forEach((module) => {
  
  
    
     if (module.battery_state) {
      modules[module.id] = module;
     /*   
      
        id: '04:00:00:6d:61:08',
        type: 'NATherm1',
        battery_state: 'very_low',
        battery_level: 2551,
        reachable: false, 
        
      */
    } 
  });

//  console.log(modules); 


  data.meta.homesdata.body.homes[0].rooms.forEach((room) => {

    
    room.module_ids.forEach((module_id) => {
      

      
      if (modules[module_id]) {


        room.battery_state = modules[module_id].battery_state;
        room.battery_level = modules[module_id].battery_level;
        room.firmware_revision = modules[module_id].firmware_revision;
        room.rf_strength = modules[module_id].rf_strength;
        room.reachable = modules[module_id].reachable;
        // room.boiler_valve_comfort_boost = modules[module_id].boiler_valve_comfort_boost;
        // room.bridge = modules[module_id].bridge;
        // room.boiler_status = modules[module_id].boiler_status;

        /* console.log(modules[module_id]); */


        // room: 
        // {
        //   id: '1812451076',
        //   name: 'Łazienka',
        //   type: 'bathroom',
        //   module_ids: [ '70:ee:50:0b:df:dc', '04:00:00:0b:df:42' ],
        //   battery_state: 'full',
        //   battery_level: 4165,
        //   firmware_revision: 79,
        //   rf_strength: 71,
        //   reachable: true
        // }
        // modules[module_id]: 
        // {
        //   id: '04:00:00:0b:df:42',
        //   type: 'NATherm1',
        //   battery_state: 'full',
        //   battery_level: 4165,
        //   firmware_revision: 79,
        //   rf_strength: 71,
        //   reachable: true,
        //   boiler_valve_comfort_boost: false,
        //   bridge: '70:ee:50:0b:df:dc',
        //   boiler_status: true
        // }

    /*    console.log("room: ");
       console.log(room);
       console.log("modules[module_id]: ");
       console.log(modules[module_id]); */


       // sprawdz czy w room[module_ids] ktoras wartosc jest rowna modules[module_id].id
        // jesli tak to dodaj do room wartosci z modules[module_id] 

        if(room.module_ids.includes(modules[module_id].id)) {
          // console.log("jest");


          room.battery_state = modules[module_id].battery_state;
          room.battery_level = modules[module_id].battery_level;
          room.rf_strength = modules[module_id].rf_strength;
          room.reachable = modules[module_id].reachable;
          
          data.rooms.forEach((roomO) => {
           
            if(roomO.id === room.id) {
              roomO.battery_state = modules[module_id].battery_state;
              roomO.battery_level = modules[module_id].battery_level;
              roomO.rf_strength = modules[module_id].rf_strength;
              roomO.reachable = modules[module_id].reachable;
            }

          }

        )


        }
     
      
      }


    }); 


    /* 
    
    modules
    {
      '04:00:00:0b:df:42': {
        id: '04:00:00:0b:df:42',
        type: 'NATherm1',
        battery_state: 'full',
        battery_level: 4269,
        firmware_revision: 79,
        rf_strength: 73,
        reachable: true,
        boiler_valve_comfort_boost: false,
        bridge: '70:ee:50:0b:df:dc',
        boiler_status: false
      },
      '04:00:00:10:14:ec': {
        id: '04:00:00:10:14:ec',
        type: 'NATherm1',
        battery_state: 'high',
        battery_level: 3879,
        firmware_revision: 79,
        rf_strength: 78,
        reachable: true,
        boiler_valve_comfort_boost: false,
        bridge: '70:ee:50:86:e1:fa',
        boiler_status: false
      },
      '04:00:00:6d:61:08': {
        id: '04:00:00:6d:61:08',
        type: 'NATherm1',
        battery_state: 'very_low',
        battery_level: 2551,
        reachable: false,
        bridge: '70:ee:50:10:2b:4a'
      },
      '04:00:00:11:a5:50': {
        id: '04:00:00:11:a5:50',
        type: 'NATherm1',
        battery_state: 'high',
        battery_level: 3787,
        firmware_revision: 79,
        rf_strength: 39,
        reachable: true,
        boiler_valve_comfort_boost: false,
        bridge: '70:ee:50:11:aa:38',
        boiler_status: false
      }
    }
    
    rooms
    
    {
      id: '1868270675',
      name: 'AE Sypialnia',
      type: 'bedroom',
      module_ids: [ '04:00:00:6d:61:08' ]
    }
    {
      id: '38038562',
      name: 'Waleria',
      type: 'custom',
      module_ids: [ '70:ee:50:11:aa:38', '04:00:00:11:a5:50' ]
    }
    {
      id: '3398980513',
      name: 'Łazienka na piętrze',
      type: 'bathroom',
      module_ids: [ '70:ee:50:10:2b:4a' ]
    }
    {
      id: '206653929',
      name: 'Kuchnia',
      type: 'kitchen',
      module_ids: [ '70:ee:50:86:e1:fa', '04:00:00:10:14:ec' ]
    }
    {
      id: '1812451076',
      name: 'Łazienka',
      type: 'bathroom',
      module_ids: [ '70:ee:50:0b:df:dc', '04:00:00:0b:df:42' ]
    } */




  });

  // Create a dictionary to map module IDs to room IDs
 /*  const module_to_room = {};
  data.homesdata.body.homes.forEach((home) => {
    home.rooms.forEach((room) => {
      room.module_ids.forEach((module_id) => {
        module_to_room[module_id] = room.id;
      });
    });
  }); */

  // // Iterate through the rooms and add the battery state
  // data.rooms.forEach((room) => {
  //   const roomId = room.id;
  //   let battery_state = null;

  //   // Find the module IDs associated with the room
  //   const module_ids_in_room = Object.keys(module_to_room).filter(
  //     (module_id) => module_to_room[module_id] === roomId
  //   );

  //   // If there are modules in the room, get the battery state of the first thermostat
  //   if (module_ids_in_room.length > 0) {
  //     const module_id = module_ids_in_room[0]; // Take the first module ID
  //     battery_state = module_battery_states[module_id] || null;
  //   }

  //   // Add the battery state to the room
  //   room.battery_state = battery_state;
  // });
// console.log(data);
  return data;
}


function filterHomestatusData(data) {
  if (!data.body.home.rooms) {
    return [];
  }

  return data.body.home.rooms.map(room => ({
    id: room.id || null,
    reachable: room.reachable || null,
    anticipating: room.anticipating || null,
    // heating_power_request: room.heating_power_request || null,
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

module.exports = { filterHomestatusData, filterHomesdata, combineRoomData , extractBatteryState};
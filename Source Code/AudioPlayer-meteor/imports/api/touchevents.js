import { Mongo } from 'meteor/mongo';
import Exhibits from './exhibits.js';
import net from 'net';

let DEBUG = false;

let EVENT_SERVER_PORT = 9000;
var eventServer;

if (Meteor.isServer) {
  initEventServer();
  
  function initEventServer() {
    eventServer = net.createServer(Meteor.bindEnvironment(function(socket) {
      socket.on('data', Meteor.bindEnvironment((data) => {
        try {
          if (!data.toString().trim()) return;
          if (DEBUG) console.log(data.toString().trim().length);
          if (DEBUG) console.log(data.toString());
          var jsonMessages = data.toString().split("][");
          jsonMessages = jsonMessages.map((item, index) => {
            if (jsonMessages.length > 1) {
              if (index == 0) return item + "]";
              if (index == jsonMessages.length-1) return "[" + item;
              return "[" + item + "]";
            }
            return item;
          });
          jsonMessages.forEach((message) => {
            if (DEBUG) console.log(JSON.parse(message)[0]);
            Meteor.call('touchevents.addEvent', JSON.parse(message)[0]);
          });
        } catch (e) {
          console.log(e);
        }
      }));
    }));
    eventServer.listen( EVENT_SERVER_PORT, () => {
      console.log("Event server listening on port "+EVENT_SERVER_PORT);
    });
    eventServer.on('error', (e) => {
      if (e.code === 'EADDRINUSE') {
        console.log('Event server address in use. Retrying...');
        setTimeout(() => {
          eventServer.close();
          eventServer.listen(EVENT_SERVER_PORT);
        }, 2000);
      }
    });
  }
}

Meteor.methods({
  'touchevents.addEvent'(payload) {
    check(payload, Object);
    check(payload.exhibitMACAddress, String);
    check(payload.deviceString, String);
    check(payload.buttonState, String);
    check(payload.sequence, Number);
    check(payload.buttonID, Number);

    Exhibits.update({
      macAddress: payload.exhibitMACAddress,
      buttons: { 
        $elemMatch: {
          id: {$eq: ''+payload.buttonID},
          sequence: {$lt: payload.sequence},
        }
      },
    },{
      $set: {
        "buttons.$.state": payload.buttonState,
        timestamp: new Date(),
        "buttons.$.sequence": payload.sequence,
      }
    });
  }
});


function extractUUID(str) {
  let match;
  if (match = str.match(/manufacturer data: (\w+?)\b/)) {
    return match[1].length > 32 ?  match[1].substring(8,40) : match[1];
  }
  else return "";
}


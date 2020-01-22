import { Mongo } from 'meteor/mongo';
import Exhibits from './exhibits.js';

export default TouchEvents = new Mongo.Collection('touchevents');

var storageWindow = 5 * 60 * 1000; // 5 minutes

if (Meteor.isServer) {
  Meteor.publish('alltouchevents', function () {
    return TouchEvents.find({});
  });
  Meteor.publish('exhibittouchevents', function (id) {
    return TouchEvents.find({exhibitMACAddress:id});
  }, {
    url: "publications/exhibittouchevents/:0",
    httpMethod: "get"
  });
  Meteor.publish('latesttouchevent', function (exhibitMACAddress, deviceUUID) {
    if (deviceUUID)
      return TouchEvents.find({exhibitMACAddress:exhibitMACAddress, deviceUUID: deviceUUID.replace(/-/g,"").toLowerCase()},{sort:{timestamp:-1},limit: 1});
    else
      return TouchEvents.find({exhibitMACAddress:exhibitMACAddress},{sort:{timestamp:-1},limit: 1});
  }, {
    url: "publications/latesttouchevent/:0",
    httpMethod: "get"
  });
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
      }
    },{
      $set: {
        "buttons.$.state": payload.buttonState,
        timestamp: new Date(),
        "buttons.$.sequence": payload.sequence,
      }
    });

    console.log(payload);

    TouchEvents.insert({
        exhibitMACAddress: payload.exhibitMACAddress,
        deviceUUID: extractUUID(payload.deviceString),
        timestamp: new Date(),
        buttonState: payload.buttonState,
        buttonID: payload.buttonID,
      });

    TouchEvents.remove({
        timestamp: { $lt: new Date(Date.now() - storageWindow) }
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


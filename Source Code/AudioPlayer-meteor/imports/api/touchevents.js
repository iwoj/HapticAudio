import { Mongo } from 'meteor/mongo';

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
  Meteor.publish('latesttouchevent', function (id) {
    return TouchEvents.find({exhibitMACAddress:id},{sort:{timestamp:-1},limit: 1});
  }, {
    url: "publications/latesttouchevent/:0",
    httpMethod: "get"
  });
}

Meteor.methods({
  'touchevents.addEvent'(payload) {
    check(payload, Object);
    check(payload.exhibitMACAddress, String);
    check(payload.buttonState, String);
    check(payload.buttonID, Number);

    TouchEvents.insert({
        exhibitMACAddress: payload.exhibitMACAddress,
        timestamp: new Date(),
        buttonState: payload.buttonState,
        buttonID: payload.buttonID,
      });

    TouchEvents.remove({
        timestamp: { $lt: new Date(Date.now() - storageWindow) }
      });
  }
});


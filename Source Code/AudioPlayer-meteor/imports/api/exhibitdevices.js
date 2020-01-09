import { Mongo } from 'meteor/mongo';

export default ExhibitDevices = new Mongo.Collection('exhibitdevices');

var storageWindow = 5 * 60 * 1000; // 5 minutes

if (Meteor.isServer) {
  Meteor.publish('allexhibitdevices', function () {
    return ExhibitDevices.find({});
  });
  Meteor.publish('exhibitdevices', function (id) {
    return ExhibitDevices.find({exhibitID:id});
  }, {
    url: "publications/exhibitdevices/:0",
    httpMethod: "get"
  });
  Meteor.publish('latestexhibitdevices', function (id) {
    return ExhibitDevices.find({exhibitID:id},{sort:{timestamp:-1},limit: 1});
  }, {
    url: "publications/latestexhibitdevices/:0",
    httpMethod: "get"
  });
}

Meteor.methods({
  'exhibitdevices.addSample'(payload) {
    check(payload, Object);
    check(payload.exhibitID, String);
    check(payload.devices, Array);

    ExhibitDevices.insert({
        exhibitID: payload.exhibitID,
        timestamp: new Date(),
        devices: extractUUID(payload.devices)
      });

    ExhibitDevices.remove({
        timestamp: { $lt: new Date(Date.now() - storageWindow) }
      });
  }
});

function extractUUID(devices) {
  for (var i = 0; i < devices.length; i++) {
    let match;
    if (match = devices[i].string.match(/manufacturer data: (\w+?)\b/)) {
      match[1].length > 32 ? devices[i].uuid = match[1].substring(8,40) : devices[i].uuid = match[1];
    }
  }
  return devices;
}

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
    return ExhibitDevices.find({exhibitMACAddress:id},{sort:{timestamp:-1},limit: 1});
  }, {
    url: "publications/latestexhibitdevices/:0",
    httpMethod: "get"
  });
  Meteor.publish('latestuserexhibitdevices', function (id) {
    // TODO:
    // Get users with registered uuids within recent window
    // Get all devices that match
    let recentWindow = 1000 * 60 * 60 * 2; // 2 hours;
    var recentUserDevices = Meteor.users.find({
      "profile.uuid" : { $exists: true},
      "profile.deviceRegisterTimestamp" : { $gt: new Date(Date.now() - recentWindow)},
    }, {
      "profile.uuid": 1
    }).fetch();
    recentUserDevices = recentUserDevices.map((user, index) => {
      return user.profile.uuid.toLowerCase().replace(/-/g, "");
    });

    console.log("recentUserDevices");
    console.log(recentUserDevices);

    //Transform function
    var transform = function(doc) {
      console.log(doc.devices);
      doc.devices = doc.devices.reduce((sum, device) => {
        console.log(device.uuid);
        if (recentUserDevices.indexOf(device.uuid) >= 0) {
          console.log("pushing");
          console.log(device);
          sum.push(device);
        }
      });
      return doc;
    }

    var self = this;

    var observer = ExhibitDevices.find({
      exhibitMACAddress:id,
    },{
      sort:{timestamp:-1},
      limit: 1,
    }).observe({
      added: function (document) {
        self.added('exhibitdevices', document._id, transform(document));
      },
      changed: function (newDocument, oldDocument) {
        self.changed('exhibitdevices', oldDocument._id, transform(newDocument));
      },
      removed: function (oldDocument) {
        self.removed('exhibitdevices', oldDocument._id);
      }
    });

    self.onStop(function () {
      observer.stop();
    });

    self.ready();
  }, {
    url: "publications/latestuserexhibitdevices/:0",
    httpMethod: "get"
  });
}

Meteor.methods({
  'exhibitdevices.addSample'(payload) {
    check(payload, Object);
    check(payload.exhibitMACAddress, String);
    check(payload.devices, Array);

    ExhibitDevices.insert({
        exhibitMACAddress: payload.exhibitMACAddress,
        timestamp: new Date(),
        devices: extractUUID(payload.devices)
      });

    ExhibitDevices.remove({
        timestamp: { $lt: new Date(Date.now() - storageWindow) }
      });
  },
  'exhibitdevices.registerdevice'(payload) {
    check(payload, Object);
    check(payload.uuid, String);

    Meteor.users.update({
      _id: Meteor.userId(),
    }, {
      $set: {
        "profile.deviceRegisterTimestamp": new Date(),
        "profile.uuid": payload.uuid,
      }
    });
  },
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



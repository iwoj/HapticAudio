import { Mongo } from 'meteor/mongo';

export default ExhibitDevices = new Mongo.Collection('exhibitdevices');

if (Meteor.isServer) {
  Meteor.publish('exhibitdevices', function exhibitDevicesPublication() {
    return ExhibitDevices.find({});
  });
}

Meteor.methods({
  'exhibitdevices.upsert'(payload) {
    check(payload, Object);
    check(payload.exhibitID, String);
    check(payload.devices, Array);

    ExhibitDevices.upsert({
        exhibitID: payload.exhibitID
      },
      {
        exhibitID: payload.exhibitID,
        devices: payload.devices
      });
  }
});


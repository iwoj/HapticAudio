import { Mongo } from 'meteor/mongo';

export default Exhibits = new Mongo.Collection('exhibits');

if (Meteor.isServer) {
  Meteor.publish('allexhibits', function () {
    return Exhibits.find({});
  });
}

Meteor.methods({
  'registerexhibit' (payload) {
    check(payload, Object);
    check(payload.macAddress, String);

    Exhibits.upsert({
      macAddress: payload.macAddress
    },
    {
      $set: {
        macAddress: payload.macAddress,
        timestamp: new Date(),
      }
    });
    console.log(Exhibits.findOne({macAddress:payload.macAddress}));
  }
});


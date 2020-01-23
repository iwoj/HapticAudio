import { Mongo } from 'meteor/mongo';

export default Exhibits = new Mongo.Collection('exhibits');

if (Meteor.isServer) {
  Meteor.publish('allexhibits', function () {
    return Exhibits.find({});
  });
  Meteor.publish('exhibit', function (id) {
    return Exhibits.find({macAddress:id});
  }, {
    url: "publications/exhibit/:0",
    httpMethod: "get"
  });
}

Meteor.methods({
  'registerexhibit' (payload) {
    check(payload, Object);
    check(payload.macAddress, String);
    check(payload.buttons, Array);

    Exhibits.upsert({
      macAddress: payload.macAddress,
    },
    {
      $set: {
        macAddress: payload.macAddress,
        buttons: payload.buttons,
        timestamp: new Date(),
      }
    });
    console.log(Exhibits.findOne({macAddress:payload.macAddress}));
  
  },
  'resetbuttonsequences' (payload) {
    check(payload, Object);
    check(payload.macAddress, String);
    Exhibits.update({
      macAddress: payload.macAddress,
      buttons : {
        $elemMatch: {
          sequence: {$gt:0},
        }
      },
    },
    {
      $set: {
        "buttons.$[].sequence": 0,
        timestamp: new Date(),
      }
    });
  }
});


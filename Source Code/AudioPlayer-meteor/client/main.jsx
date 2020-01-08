import React from 'react';
import { Meteor } from 'meteor/meteor';
import { render } from 'react-dom';
import App from '/imports/ui/App'

Meteor.startup(() => {
  console.log(Meteor.isCordova ? "Device UUID: " + device.uuid: "");
  render(<App />, document.getElementById('react-target'));
});

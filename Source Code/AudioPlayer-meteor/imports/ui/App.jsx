import React, { Component } from 'react';
import ReactDOM from 'react-dom';
import { Meteor } from 'meteor/meteor';
import { withTracker } from 'meteor/react-meteor-data';
import ExhibitDevices from '../api/exhibitdevices.js';
import TouchEvents from '../api/touchevents.js';

class App extends Component {
  constructor(props) {
    super(props);

    this.state = {
      hideCompleted: false,
    };
  }

  renderDevices() {
    let devices = this.props.devices;
    return devices.map((device, index) => {
      return (
        <li key={device.address} className={index == 0 ? "closestDevice" : ""}>{device.uuid} {device.signalStrength}</li>
      );
    });
  }

  render() {
    return (
      <div className="container">
        <header>
          <h1>{this.props.deviceCount} Nearby Device{this.props.deviceCount == 1 ? "" : "s"}</h1>
        </header>
        <ul>
          {this.renderDevices()}
        </ul>
      </div>
    );
  }
}

export default withTracker(() => {
  Meteor.subscribe('latestexhibitdevices', 'testExhibit1');

  return {
    devices: ExhibitDevices.find().fetch().length > 0 ? ExhibitDevices.find().fetch()[0].devices : [],
    deviceCount: ExhibitDevices.find().fetch().length > 0 ? ExhibitDevices.find().fetch()[0].devices.length : 0,
  };
})(App);


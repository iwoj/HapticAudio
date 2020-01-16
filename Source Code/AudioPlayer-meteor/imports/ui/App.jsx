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
  

  // componentDidMount() {
  //   AccountsAnonymous.login(() => {
  //     Meteor.call("globalsettings.registerUser", {uuid: device.uuid, `});
  //   });
  // }
  

  renderDevices() {
    let devices = this.props.devices;
    return devices.map((currentDevice, index) => {
      return (
        <li key={currentDevice.address} className={index == 0 ? "closestDevice" : ""}>{currentDevice.uuid ? currentDevice.uuid : currentDevice.address} {currentDevice.signalStrength}</li>
      );
    });
  }
  
  isClosest() {
    let devices = this.props.devices;
    let closest = false;
    devices.forEach((currentDevice, index) => {
      if (index == 0 && Meteor.isCordova && this.uuidMatch(device.uuid, currentDevice.uuid)) {
        closest = true;
      }
    });
    return closest;
  }

  uuidMatch(uuid1, uuid2) {
    if (!uuid1 || !uuid2) return;
    if (uuid1.replace(/-/g,"").toLowerCase() == uuid2.replace(/-/g,"").toLowerCase())
      return true;
    else 
      return false;
  }

  whichTouchDown() {
    if (this.props.touchEvent && this.props.touchEvent.buttonState == "down")
      return this.props.touchEvent.buttonID;
  }
  
  render() {
    let className = "container";
    this.isClosest() ? className += " iAmClosest" : "";
    this.whichTouchDown() ? className += " button" + this.whichTouchDown() + "Down" : "";
    return (
      <div className={className}>
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
  Meteor.subscribe('latestexhibitdevices', '30:ae:a4:58:42:48');
  Meteor.subscribe('latesttouchevent', '30:ae:a4:58:42:48', typeof device !== 'undefined'  ? device.uuid : null);
  
  return {
    devices: ExhibitDevices.find().fetch().length > 0 ? ExhibitDevices.find().fetch()[0].devices : [],
    deviceCount: ExhibitDevices.find().fetch().length > 0 ? ExhibitDevices.find().fetch()[0].devices.length : 0,
    touchEvent: TouchEvents.findOne(),
  };
})(App);


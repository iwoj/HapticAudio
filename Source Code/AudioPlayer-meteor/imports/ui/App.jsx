import React, { Component } from 'react';
import ReactDOM from 'react-dom';
import { Meteor } from 'meteor/meteor';
import { withTracker } from 'meteor/react-meteor-data';
import ExhibitDevices from '../api/exhibitdevices.js';
import Exhibits from '../api/exhibits.js';

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
  
  renderDevices() {
    let devices = this.props.devices;
    return devices.map((currentDevice, index) => {
      return (
        <li key={currentDevice.address} className={index == 0 ? "closestDevice" : ""}>{currentDevice.uuid ? currentDevice.uuid : currentDevice.address} {currentDevice.signalStrength}</li>
      );
    });
  }
  
  renderButtons() {
    let exhibit = this.props.exhibit;
    if (!(exhibit && exhibit.buttons)) return "";
    return exhibit.buttons.map((currentButton, index) => {
      return (
        <li key={currentButton.id}>{currentButton.id} ({currentButton.sequence}): {currentButton.state}</li>
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

  isButtonDown(id) {
    let buttonDown = false;
    if (this.props.exhibit && this.props.exhibit.buttons) {
      for (let i = 0; i < this.props.exhibit.buttons.length; i++) {
        if (this.props.exhibit.buttons[i].id == id && this.props.exhibit.buttons[i].state == "down")
          buttonDown = true;
      }
      return buttonDown;
    }
    else {
      return false;
    }
  }
  
  render() {
    let className = "container";
    this.isClosest() ? className += " iAmClosest" : "";
    this.isButtonDown("1") ? className += " button1Down" : "";
    this.isButtonDown("2") ? className += " button2Down" : "";
    this.isButtonDown("3") ? className += " button3Down" : "";
    return (
      <div className={className}>
        <header>
          <h1>{this.props.deviceCount} Nearby Device{this.props.deviceCount == 1 ? "" : "s"}</h1>
        </header>
        <ul>
          {this.renderDevices()}
        </ul>
        <ul>
          {this.renderButtons()}
        </ul>
      </div>
    );
  }
}

export default withTracker(() => {
  Meteor.subscribe('latestexhibitdevices', '30:ae:a4:58:42:48');
  Meteor.subscribe('exhibit', '30:ae:a4:58:42:48');
  
  return {
    devices: ExhibitDevices.find().fetch().length > 0 ? ExhibitDevices.find().fetch()[0].devices : [],
    deviceCount: ExhibitDevices.find().fetch().length > 0 ? ExhibitDevices.find().fetch()[0].devices.length : 0,
    exhibit: Exhibits.findOne(),
  };
})(App);


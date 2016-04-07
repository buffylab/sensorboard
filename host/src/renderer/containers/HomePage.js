import React, { Component } from 'react';
import { connect } from 'react-redux';
import Paper from 'material-ui/lib/paper';
import Home from '../components/Home';

const styles = {
  container: {
    display: 'flex',
    flexDirection: 'row',
  },
  item: {
    width: 200,
    height: 300,
  },
};

@connect(state => ({
  usbError: state.usb.error,
  usbDevices: state.usb.devices,
}))
class HomePage extends Component {
  renderBody() {
    if (this.props.usbError) {
      return <div>{this.props.usbError}</div>;
    }

    const list = Object.keys(this.props.usbDevices).map(id => {
      const device = this.props.usbDevices[id];
      return (
        <Paper style={styles.item} key={id}>
          <div>{id}</div>
          <div>{device.manufacturer}</div>
          <div>{device.product}</div>
          <div>{device.serial_number}</div>
        </Paper>
      );
    });

    return <div style={styles.container}>{list}</div>;
  }

  render() {
    return <div>
      <div>List of USB Devices</div>
      {this.renderBody()}
    </div>;
  }
}

export default HomePage;

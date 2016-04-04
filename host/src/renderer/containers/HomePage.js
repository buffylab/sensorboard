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
    width: 100,
    height: 100,
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
          {id}
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

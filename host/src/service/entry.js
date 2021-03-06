import express from 'express';
import http from 'http';
import Io from 'socket.io';
import bindings from 'bindings';
import { crashReporter } from 'electron';
import shortid from 'shortid';

if (__DEV__) {
  console.log(`service process start (pid=${process.pid})`);

  if (process.env.LMDS /* Local mini dump server */) {
    console.log(`Start crash reporter (target: ${process.env.LMDS})`);
    crashReporter.start({
        productName: 'YourName',
        companyName: 'YourCompany',
        submitURL: `${process.env.LMDS}/post`,
        autoSubmit: true,
    });
  }
}

const pkg = require('../../package.json');

const android = bindings('android');

const app = express();
const server = http.Server(app);
const io = new Io(server);

function serializeDeviceUpdateEvent(error, devices) {
  return {
    error: error ? error.message : '',
    devices,
  };
}

io.on('connection', socket => {
  android.getUsbDeviceState((error, devices) => {
    socket.emit('usb', serializeDeviceUpdateEvent(error, devices));
  });
});

app.get('/', (req, res) => {
  res.send({
    name: pkg.name,
    version: pkg.version,
  });
});

server.listen(7000);
console.log(`Listening at ${7000}`);

const usbBuffers = {};

android.start(
  // Handle device info update
  function(error, devices) {
    io.emit('usb', serializeDeviceUpdateEvent(error, devices));
  },

  // Handle incoming data
  function (devId, data) {
    const buffer = (usbBuffers[devId] || '') + data;

    let fromIndex = 0;
    let index;

    while((index = buffer.indexOf(';', fromIndex)) !== -1) {
      const chunk = buffer.substr(fromIndex, index - fromIndex);
      fromIndex = index + 1;

      try {
        const json = JSON.parse(chunk)
        const id = shortid.generate();
        io.emit('usb:event', { devId, event: { id, data: json } });
      } catch(err) {
        console.error(err);
      }
    }

    // Store unsent data
    usbBuffers[devId] = buffer.substr(fromIndex);
  }
);

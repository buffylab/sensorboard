import express from 'express';
import http from 'http';
import Io from 'socket.io';
import bindings from 'bindings';

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

android.start(
  // Handle device info update
  function(error, devices) {
    io.emit('usb', serializeDeviceUpdateEvent(error, devices));
  },

  // Handle incoming data
  function (id, data) {

  }
);

process.on('uncaughtException', (error) => {
  console.error(error.stack);
  process.exit(1);
});

import io from 'socket.io-client';
import {
  updateDeviceState,
  receiveUsbEvent,
} from '../actions/usb';

const socket = io('http://localhost:7000');

export default function routeEvents(store) {
  socket.on('usb', ({ error, devices }) => {
    store.dispatch(updateDeviceState(error, devices));
  });

  socket.on('usb:event', ({ devId, event }) => {
    store.dispatch(receiveUsbEvent(devId, event));
  });
};
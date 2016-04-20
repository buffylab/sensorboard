import io from 'socket.io-client';
import {
  ioConnect,
  ioDisconnect,
  buttonDown,
  buttonUp,
} from '../actions';

export default function configureSocket(store) {
  const socket = io('http://localhost:7000');

  socket.on('connect', () => {
    store.dispatch(ioConnect());
  });

  socket.on('disconnect', () => {
    store.dispatch(ioDisconnect());
  });

  socket.on('usb', ({ error, devices }) => {
    // store.dispatch(updateDeviceState(error, devices));
  });

  socket.on('usb:event', ({ devId, event }) => {
    // TODO: Use button up and down events
    store.dispatch(buttonDown());
    setTimeout(() => store.dispatch(buttonUp()), 30);
  });

  return socket;
}

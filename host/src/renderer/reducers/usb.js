import update from 'react-addons-update';
import {
  USB_DEVICE_UPDATE,
  USB_DEVICE_RX_EVENT,
} from '../actions/usb';

const MAX_EVENT_LENGTH = 5;

const initialState = {
  error: '',
  devices: {},
  events: {},
};

export default function usb(state = initialState, action) {
  switch (action.type) {
    case USB_DEVICE_UPDATE:
      return update(state, {
        error: { $set: action.error },
        devices: { $set: action.devices },
      });
    case USB_DEVICE_RX_EVENT:
      const events = state.events[action.devId] || [];
      const nextEvents = events.length < MAX_EVENT_LENGTH
        ? [action.event].concat(events)
        : [action.event].concat(events.slice(0, MAX_EVENT_LENGTH - 1));

      return update(state, {
        events: {
          [action.devId]: { $set: nextEvents },
        },
      });
    default:
      return state;
  }
}

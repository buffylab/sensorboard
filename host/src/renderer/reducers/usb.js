import {
  USB_DEVICE_UPDATE,
} from '../actions/usb';

const initialState = {
  error: '',
  devices: {},
};

export default function usb(state = initialState, action) {
  switch (action.type) {
    case USB_DEVICE_UPDATE:
      return {
        error: action.error,
        devices: action.devices,
      };
    default:
      return state;
  }
}

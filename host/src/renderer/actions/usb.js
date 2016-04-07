export const USB_DEVICE_UPDATE = 'USB_DEVICE_UPDATE';
export function updateDeviceState(error, devices) {
  return {
    type: USB_DEVICE_UPDATE,
    error,
    devices,
  };
}

export const USB_DEVICE_RX_EVENT = 'USB_DEVICE_RX_EVENT';
export function receiveUsbEvent(devId, event) {
  return {
    type: USB_DEVICE_RX_EVENT,
    devId, event,
  };
}

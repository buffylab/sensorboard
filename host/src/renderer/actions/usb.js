export const USB_DEVICE_UPDATE = 'USB_DEVICE_UPDATE';

export function updateDeviceState(error, devices) {
  return {
    type: USB_DEVICE_UPDATE,
    error,
    devices,
  };
}

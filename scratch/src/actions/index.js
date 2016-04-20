export const IO_CONNECT = 'IO_CONNECT';
export function ioConnect() {
  return { type: IO_CONNECT };
}

export const IO_DISCONNECT = 'IO_DISCONNECT';
export function ioDisconnect() {
  return { type: IO_DISCONNECT };
}

export const BUTTON_DOWN = 'BUTTON_DOWN';
export function buttonDown() {
  return { type: BUTTON_DOWN };
}

export const BUTTON_UP = 'BUTTON_UP';
export function buttonUp() {
  return { type: BUTTON_UP };
}

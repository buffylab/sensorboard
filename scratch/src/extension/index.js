import createSocket from '../socket';

function createExt(getState, onShutdown) {
  function _getStatus() {
    const { io } = getState();
    return io.connected
      ? { status: 2, msg: 'Connected' }
      : { status: 1, msg: 'Waiting' };
  }

  return {
    _getStatus,
    _shutdown: () => onShutdown(),
  };
}

export default function createExtension(store) {
  const socket = createSocket(store);

  function handleShutdown() {
    socket.close();
  }

  const descriptor = { blocks: [] };
  const ext = createExt(() => store.getState(), handleShutdown);

  return { descriptor, ext };
}

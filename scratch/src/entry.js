const { ScratchExtensions } = window;
import configureStore from './store';
import createExtension from './extension';
import installBlocks from './blocks';

const EXTENSION_NAME = 'Sensorboard extension';

const store = configureStore();

const { descriptor, ext } = createExtension(store);
installBlocks(store, descriptor, ext);

ScratchExtensions.register(EXTENSION_NAME, descriptor, ext);

if (module.hot) {
  module.hot.accept([
    './extension',
    './blocks',
    './socket',
  ], () => {
    const nextCreateExtension = require('./extension').default;
    const nextInstallBlocks = require('./blocks').default;

    const { descriptor: nextDescriptor, ext: nextExt } = createExtension(store);
    nextInstallBlocks(store, nextDescriptor, nextExt);

    ScratchExtensions.unregister(EXTENSION_NAME);
    ScratchExtensions.register(EXTENSION_NAME, nextDescriptor, nextExt);
  })
}

import whenButtonClick from './whenButtonClick';

export default function installBlocks(store, descriptor, ext) {
  const getState = () => store.getState();
  const dispatch = action => store.dispatch(action);

  const blocks = [
    whenButtonClick(getState, dispatch),
  ];
  blocks.forEach(block => block.install(descriptor, ext));
}

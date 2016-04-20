import invariant from 'invariant';
import createBlock from './createBlock';

export default function (getState, dispatch) {
  let buttonPressedAt = 0;

  function handler() {
    try {
      const { button } = getState();
      if (button.pressed) {
        buttonPressedAt = Date.now();
        return false;
      } else {
        const clicked = Date.now() - buttonPressedAt < 100;
        buttonPressedAt = 0;
        return clicked;
      }
    } catch(error) {
      console.error(error.stack || error);
    }

    return false;
  }

  return createBlock({
    handler,
    opCode: 'h',
    label: 'when button click!',
    method: 'whenButtonClick',
  });
};

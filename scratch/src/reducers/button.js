import objectAssign from 'object-assign';

import {
  BUTTON_DOWN,
  BUTTON_UP,
} from '../actions';

const initialState = {
  pressed: false,
};

export default function button(state = initialState, action) {
  switch(action.type) {
    case BUTTON_DOWN: {
      return objectAssign({}, state, { pressed: true });
    }
    case BUTTON_UP: {
      return objectAssign({}, state, { pressed: false });
    }
  }
  return state;
};

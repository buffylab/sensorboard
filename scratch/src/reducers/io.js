import objectAssign from 'object-assign';

import {
  IO_CONNECT,
  IO_DISCONNECT,
} from '../actions';

const initialState = {
  connected: false,
};

export default function io(state = initialState, action) {
  switch(action.type) {
    case IO_CONNECT: {
      return objectAssign({}, state, { connected: true });
    }
    case IO_DISCONNECT: {
      return objectAssign({}, state, { connected: false });
    }
  }
  return state;
};

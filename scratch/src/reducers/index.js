import { combineReducers } from 'redux';
import io from './io';
import button from './button';

const rootReducer = combineReducers({
  io,
  button,
})

export default rootReducer;

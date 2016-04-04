import { combineReducers } from 'redux';
import { routerReducer as routing } from 'react-router-redux';
import usb from './usb';

const rootReducer = combineReducers({
  routing,
  usb,
});

export default rootReducer;

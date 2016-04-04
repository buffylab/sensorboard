require('react-tap-event-plugin')();

import React from 'react';
import { render } from 'react-dom';
import { Provider } from 'react-redux';
import { Router, hashHistory } from 'react-router';
import { syncHistoryWithStore } from 'react-router-redux';
import $script from 'scriptjs';
require('roboto-fontface/css/roboto-fontface.css');

import routes from './routes';
import configureStore from './store/configureStore';
require('./styles.css');

import { updateDeviceState } from './actions/usb';

const store = configureStore();
const history = syncHistoryWithStore(hashHistory, store);

$script('http://localhost:7000/socket.io/socket.io.js', () => {
  const socket = io('http://localhost:7000');
  socket.on('usb', ({ error, devices }) => {
    store.dispatch(updateDeviceState(error, devices));
  });
});

render(
  <Provider store={store}>
    <Router history={history} routes={routes} />
  </Provider>,
  document.getElementById('root')
);

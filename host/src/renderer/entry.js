require('react-tap-event-plugin')();

import React from 'react';
import { render } from 'react-dom';
import { Provider } from 'react-redux';
import { Router, hashHistory } from 'react-router';
import { syncHistoryWithStore } from 'react-router-redux';
require('roboto-fontface/css/roboto-fontface.css');

import routes from './routes';
import configureStore from './store/configureStore';
require('./styles.css');

const store = configureStore();
const history = syncHistoryWithStore(hashHistory, store);

render(
  <Provider store={store}>
    <Router history={history} routes={routes} />
  </Provider>,
  document.getElementById('root')
);

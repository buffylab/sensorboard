import React from 'react';
import { Route, IndexRoute, Redirect } from 'react-router';
import App from './containers/App';
import HomePage from './containers/HomePage';

export default (
  <Route path="/" component={App}>
    <IndexRoute component={HomePage} />
    <Redirect from="*" to="/" />
  </Route>
);

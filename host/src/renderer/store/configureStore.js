import { createStore, applyMiddleware, compose } from 'redux';
import { persistState } from 'redux-devtools';
import thunk from 'redux-thunk';
import createLogger from 'redux-logger';
import { hashHistory } from 'react-router';
import { routerMiddleware } from 'react-router-redux';
import routeEvents from '../io';
import rootReducer from '../reducers';
import DevTools from '../containers/DevTools';

const router = routerMiddleware(hashHistory);
let enhancer;

if (__DEV__) {
  const logger = createLogger({
    level: 'info',
    collapsed: true,
  });

  enhancer = compose(
    applyMiddleware(thunk, router, logger),
    DevTools.instrument(),
    persistState(window.location.href.match(/[?&]debug_session=([^&]+)\b/))
  );
} else {
  enhancer = applyMiddleware(thunk, router);
}

export default function configureStore(initialState) {
  const store = createStore(rootReducer, initialState, enhancer);
  routeEvents(store);

  if (__DEV__ && module.hot) {
    module.hot.accept('../reducers', () =>
      store.replaceReducer(require('../reducers'))
    );
  }

  return store;
}

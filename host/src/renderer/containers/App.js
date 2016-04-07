import React, { Component, PropTypes } from 'react';

export default class App extends Component {
  static propTypes = {
    children: PropTypes.element.isRequired
  };

  render() {
    return (
      <div>
        {this.props.children}
        {
          (() => {
            if (__DEV__) {
              const { default: DevTools } = require('./DevTools');
              return <DevTools />;
            }
          })()
        }
      </div>
    );
  }
}

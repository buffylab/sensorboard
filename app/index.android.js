/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 */

import React, {
  AppRegistry,
  Component,
  StyleSheet,
  Text,
  View,
  TouchableHighlight,
  NativeModules,
} from 'react-native';

const { Usb } = NativeModules;

class SensorboardApp extends Component {
  handleButtonClick() {
    Usb.send(JSON.stringify({ message: 'Hello from React Native!' }));
  }

  render() {
    return (
      <View style={styles.container}>
        <Text style={styles.welcome}>
          Welcome to React Native!
        </Text>
        <Text style={styles.instructions}>
          To get started, edit index.android.js
        </Text>
        <Text style={styles.instructions}>
          Shake or press menu button for dev menu
        </Text>
        <TouchableHighlight onPress={() => this.handleButtonClick()}
                            activeOpacity={75 / 100}
                            underlayColor={"rgb(110, 110, 110)"}
                            style={styles.button}
        >
          <Text>Press</Text>
        </TouchableHighlight>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
  welcome: {
    fontSize: 20,
    textAlign: 'center',
    margin: 10,
  },
  instructions: {
    textAlign: 'center',
    color: '#333333',
    marginBottom: 5,
  },
  button: {
    backgroundColor: '#ccc',
    borderRadius: 5,
    padding: 10,
    marginBottom: 10
  },
});

AppRegistry.registerComponent('SensorboardApp', () => SensorboardApp);

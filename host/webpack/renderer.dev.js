const webpack = require('webpack');
const webpackTargetElectronRenderer = require('webpack-target-electron-renderer');
const _ = require('lodash');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const ExtractTextPlugin = require("extract-text-webpack-plugin");

const config = require('../config');

const root = `${__dirname}/..`;

const babelOptions = JSON.stringify({
  presets: [
    'es2015',
    'stage-0',
    'react',
    'react-hmre',
  ],
  plugins: [
    'syntax-decorators',
    'transform-decorators-legacy',
    'syntax-async-functions',
    'transform-regenerator',
    'syntax-object-rest-spread',
    'transform-object-rest-spread',
  ],
  babelrc: false,
});

const webpackConfig = {
  target: 'web',

  entry: [
    'webpack-hot-middleware/client?path=http://localhost:3000/__webpack_hmr',
    `${root}/src/renderer/entry`,
  ],

  module: {
    loaders: [
      { test: /\.json$/, loader: 'json' },
      { test: /\.jsx?$/, loader: `babel?${babelOptions}`, exclude: /node_modules/ },
      { test: /\.css$/, loader: ExtractTextPlugin.extract('style', 'css') },
      { test: /\.woff(2)?(\?v=[0-9]\.[0-9]\.[0-9])?$/,
        loader: 'url?limit=10000&minetype=application/font-woff' },
      { test: /\.(ttf|eot|svg)(\?v=[0-9]\.[0-9]\.[0-9])?$/, loader: 'file' },
    ],
  },

  output: {
    path: `${root}/out/development`,
    filename: 'renderer.js',
    publicPath: `http://localhost:${config.rendererBuildServerPort}/`,
    libraryTarget: 'commonjs2',
  },

  plugins: [
    new ExtractTextPlugin('styles.css'),
    new webpack.HotModuleReplacementPlugin(),
    new webpack.NoErrorsPlugin(),
    new webpack.DefinePlugin(_.mapValues({
      'process.env.NODE_ENV': 'development',
      __DEV__: true,
    }, val => JSON.stringify(val))),
    new HtmlWebpackPlugin({
      template: `${root}/src/renderer/index.html`, // Load a custom template
      filename: 'index.html',
    }),
  ],

  debug: true,
  devtool: 'cheap-module-eval-source-map',
};

module.exports = webpackConfig;

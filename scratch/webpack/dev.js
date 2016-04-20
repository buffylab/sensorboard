const webpack = require('webpack');

const root = `${__dirname}/..`;

const port = require('../config').DEV_PORT;

module.exports = {
  entry: [
    `webpack-hot-middleware/client?path=http://localhost:${port}/__webpack_hmr`,
    `${root}/src/entry.js`,
  ],
  output: {
    path: `${root}/dist`,
    filename: 'extension.js',
    publicPath: `http://localhost:${port}/dist/`,
  },
  module: {
    loaders: [
      { test: /\.js$/, loader: 'babel', exclude: /node_modules/ },
    ],
  },
  plugins: [
    new webpack.optimize.OccurenceOrderPlugin(),
    new webpack.HotModuleReplacementPlugin(),
  ],
  devtool: 'cheap-module-eval-source-map',
};

const webpack = require('webpack');
const _ = require('lodash');

const root = `${__dirname}/..`;

const babelOptions = JSON.stringify({
  presets: [
    'es2015',
  ],
  plugins: [
    'syntax-async-functions',
    'transform-regenerator',
    'syntax-object-rest-spread',
    'transform-object-rest-spread',
  ],
  babelrc: false,
});

module.exports = {
  target: 'electron',

  entry: `${root}/src/main/entry.js`,

  output: {
    path: `${root}/out/development`,
    filename: 'main.js',
    libraryTarget: 'commonjs2',
  },

  module: {
    loaders: [
      { test: /\.json$/, loader: 'json-loader' },
      { test: /\.jsx?$/, loader: `babel-loader?${babelOptions}`, exclude: /node_modules/ },
    ],
  },

  externals(context, request, cb) {
    const isExternal =
      request.match(/^[@a-z][a-z\/\.\-0-9]*$/i);
      cb(null, Boolean(isExternal));
  },

  plugins: [
    new webpack.DefinePlugin(_.mapValues({
      'process.env.NODE_ENV': 'development',
      __DEV__: true,
    }, val => JSON.stringify(val))),

    new webpack.BannerPlugin('require("source-map-support").install();', {
      raw: true, entryOnly: false,
    }),
  ],

  node: {
    __dirname: false,
  },

  devtool: 'source-map',
};

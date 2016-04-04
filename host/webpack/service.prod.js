const update = require('react-addons-update');
const webpack = require('webpack');
const _ = require('lodash');

const devConfig = require('./service.dev');

const root = `${__dirname}/..`;

module.exports = update(devConfig, {
  output: {
    path: { $set: `${root}/out/production` },
  },

  plugins: { $set: [
    new webpack.DefinePlugin(_.mapValues({
      'process.env.NODE_ENV': 'production',
      __DEV__: false,
    }), val => JSON.stringify(val)),

    new webpack.BannerPlugin('require("source-map-support").install();', {
      raw: true, entryOnly: false,
    }),

    new webpack.optimize.UglifyJsPlugin({
      compressor: { warnings: false },
    }),
  ] },
});

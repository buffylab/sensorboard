import express from 'express';
import webpack from 'webpack';

export default function runHotServer (port, webpackConfig) {
  return new Promise((resolve, reject) => {
    const app = express();
    const compiler = webpack(webpackConfig);

    app.use(require('webpack-dev-middleware')(compiler, {
      publicPath: webpackConfig.output.publicPath,
      stats: {
        colors: true,
      },
      noInfo: true,
    }));
    app.use(require('webpack-hot-middleware')(compiler));

    app.listen(port, err => err ? reject(err) : resolve());
  });
};

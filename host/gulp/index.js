import gulp from 'gulp';
import gutil from 'gulp-util';
import webpack from 'webpack';
import notifier from 'node-notifier';
import superb from 'superb'
import { Childminder } from 'childminder';
import {
  rendererBuildServerPort,
} from '../config';

function getCompileError(statsJson) {
  const { errors, warnings } = statsJson;
  if(errors.length > 0) {
    return new Error(statsJson.errors.join('\n'));
  } else if(warnings.length > 0) {
    return new Error(statsJson.warnings.join('\n'));
  }
  return null;
}

function notifyCompileResult(name, statsJson) {
  const { errors, warnings } = statsJson;
  if (errors.length > 0) {
    errors.forEach(error => console.error(error));
    notifier.notify({
      title: `Build ${name} failed`,
      message: errors.join('\n'),
    });
  } else if (warnings.length > 0) {
    warnings.forEach(warings => console.error(warnings));
    notifier.notify({
      title: `Build ${name} failed`,
      message: warnings.join('\n'),
    });
  } else {
    notifier.notify({
      title: `Build ${name} succeeded`,
      message: `${superb()}!`,
    });
  }
}

const cm = new Childminder();
const electronBinary = `${__dirname}/../node_modules/.bin/electron`;

gulp.task('run:renderer', () => {
  const { default: runHotServer } = require('./runHotServer');
  const config = require('../webpack/renderer.dev');
  return runHotServer(rendererBuildServerPort, config);
});

gulp.task('run:main', ['run:renderer'], async () => {
  const config = require('../webpack/main.dev');
  const compiler = webpack(config);

  await new Promise(resolve => {
    compiler.watch({}, (err, stats) => {
      const statsJson = stats.toJson();
      const error = err || getCompileError(statsJson);
      if (!error) { resolve(); }
      notifyCompileResult('main', statsJson);
    });
  });

  const output = `${config.output.path}/${config.output.filename}`;
  const child = cm.create(electronBinary, [output], { lazy: true });

  child.startOrRestart();

  gulp.watch(output).on('change', () => {
    gutil.log('Restart main...');
    child.startOrRestart();
  });
});

gulp.task('run:service', async () => {
  const config = require('../webpack/service.dev');
  const compiler = webpack(config);

  await new Promise(resolve => {
    compiler.watch({}, (err, stats) => {
      const statsJson = stats.toJson();
      const error = err || getCompileError(statsJson);
      if (!error) { resolve(); }
      notifyCompileResult('service', statsJson);
    });
  });

  const output = `${config.output.path}/${config.output.filename}`;
  const child = cm.create(electronBinary, [output], {
    env: {
      ELECTRON_RUN_AS_NODE: true,
    },
  });

  child.terminal.on('exit', function () {
    console.log(arguments);
  });

  gulp.watch(output).on('change', () => {
    gutil.log('Restart service...');
    child.startOrRestart();
  });
});

gulp.task('run', ['run:service', 'run:main'], () => {
  console.log('run!');
});

import gulp from 'gulp';
import gutil from 'gulp-util';
import path from 'path';
import os from 'os';
import webpack from 'webpack';
import notifier from 'node-notifier';
import superb from 'superb'
import { spawn } from 'child_process';
import {
  root,
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

gulp.task('build:addon', done => {
  const gyp = path.resolve(
    path.dirname(require.resolve('npm/package.json')),
    `bin/node-gyp-bin/${os.platform() === 'win32' ? 'node-gyp.cmd' : 'node-gyp'}`
  );
  const pkg = require('../package.json');
  spawn(gyp, [
    'rebuild',
    `--target=${pkg.devDependencies['electron-prebuilt']}`,
    '--dist-url=https://atom.io/download/atom-shell',
  ], { 
    stdio: 'inherit',
    env: { HOME: path.resolve(__dirname, '../.electron-gyp') },
  }).on('close', code => code === 0 ? done() : done(new Error(`exit code = %{code}`)));
});

const electronBinary = path.resolve(root, 
  os.platform() === 'win32' ? 'node_modules/.bin/electron.cmd' : 'node_modules/.bin/electron'
);

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

  const output = path.join(config.output.path, config.output.filename);
  let child;

  function restartProcess() {
    if (child) child.kill();
    child = spawn(electronBinary, [output], { stdio: 'inherit' });
  }
  restartProcess();

  gulp.watch(output).on('change', () => {
    gutil.log('Restart main...');
    restartProcess();
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
  let child;

  function restartProcess() {
    if (child) child.kill();
    child = spawn(electronBinary, [output], {
      env: Object.assign({}, process.env, { 
        ELECTRON_ENABLE_STACK_DUMPING: true,
      }),
      stdio: 'inherit',
    });
    child.on('exit', code => { 
      console.log(`service process exit (code = ${code})`);
    });
  }
  restartProcess();

  gulp.watch(output).on('change', () => {
    gutil.log('Restart service...');
    restartProcess();
  });
});

gulp.task('run', ['run:service', 'run:main'], () => {
  console.log('run!');
});

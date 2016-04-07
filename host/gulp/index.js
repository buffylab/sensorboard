import gulp from 'gulp';
import gutil from 'gulp-util';
import path from 'path';
import os from 'os';
import webpack from 'webpack';
import notifier from 'node-notifier';
import superb from 'superb'
import { spawn } from 'child_process';
import kill from 'tree-kill';
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

function buildAddon(debug, done) {
  const gyp = path.resolve(
    path.dirname(require.resolve('npm/package.json')),
    `bin/node-gyp-bin/${os.platform() === 'win32' ? 'node-gyp.cmd' : 'node-gyp'}`
  );
  const pkg = require('../package.json');
  const child = spawn(gyp, [
    'rebuild',
    `--target=${pkg.devDependencies['electron-prebuilt']}`,
    '--dist-url=https://atom.io/download/atom-shell',
  ].concat(debug ? ['--debug'] : []), {
    stdio: 'inherit',
    cwd: path.resolve(__dirname, '..'),
    env: Object.assign({}, process.env, {
      HOME: path.resolve(__dirname, '../.electron-gyp'),
    }),
  }).on('close', code => code === 0 ? done() : done(new Error(`exit code = ${code}`)));
}

class Proc {
  constructor(label, script) {
    this.label = label;
    this.script = script;
    this.restarting = false;
    this.restartReserved = false;
    this.child = null;
  }


  async restart() {
    try {
      if (this.restarting) {
        this.restartReserved = true;
        return;
      }

      this.restarting = true;
      this.restartReserved = false;

      if (this.child) {
        await new Promise((resolve, reject) => {
          kill(this.child.pid, 'SIGKILL', err => err ? reject(err) : resolve());
        });
        this.child = null;
      }

      this.child = spawn(Proc.bin, [this.script], {
        env: Object.assign({}, process.env, {
          ELECTRON_ENABLE_STACK_DUMPING: true,
        }),
      });
      this.child.stdout.on('data', data => process.stdout.write(data));
      this.child.stderr.on('data', data => process.stderr.write(data));

      this.child.on('exit', code => {
        console.log(`${this.label} process exit (code = ${code})`);
      });
    } catch(err) {
      console.error(`Restart failed: ${err}`);
    }

    this.restarting = false;

    if (this.restartReserved) {
      return restart();
    }
  }
}

Proc.bin = path.resolve(root,
  os.platform() === 'win32' ? 'node_modules/.bin/electron.cmd' : 'node_modules/.bin/electron'
);

gulp.task('build:addon:debug', done => buildAddon(true, done));
gulp.task('build:addon:release', done => buildAddon(false, done));

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

  const proc = new Proc('main', output);
  proc.restart();

  gulp.watch(output).on('change', () => {
    gutil.log('Restart main...');
    proc.restart();
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

  const proc = new Proc('service', output);
  proc.restart();

  gulp.watch(output).on('change', () => {
    gutil.log('Restart service...');
    proc.restart();
  });
});

gulp.task('run', ['run:service', 'run:main'], () => {
  console.log('run!');
});

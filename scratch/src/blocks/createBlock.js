import invariant from 'invariant';

const OpCodes = [
  ' ', 'w', 'r', 'R', 'h',
];

function createBlock({ opCode, label, method, handler, args }) {
  invariant(OpCodes.indexOf(opCode) !== -1,
    `invalid opCode ${opCode}. opCode must be one of these options: ['${OpCodes.join('\', \'')}']`
  );
  invariant(typeof label === 'string', `label is required`);
  invariant(typeof method === 'string', `method is required`);
  invariant(typeof handler === 'function', `handler is required`);

  function install(descriptor, ext) {
    ext[method] = handler;
    descriptor.blocks.push([ opCode, label, method ].concat(args || []));
  }

  return {
    install,
  }
}

export default createBlock;

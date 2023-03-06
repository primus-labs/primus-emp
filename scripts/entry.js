var arguments = process.argv;
// console.log(arguments);

script_name = './test_' + arguments[2] + '.js';
console.log('run ' + script_name);
const Module = require(script_name);
// console.log('Module ', Module);

Module.onRuntimeInitialized = () => {
  console.log("Module Initialized OK");
};


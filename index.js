const addon = require('./build/Release/sQeeZ-Parser-Node');
const parser = new addon.ParserNode(require);

console.log(addon.pingParser());
console.log(parser.pingInstance());
console.log(addon.info());
console.log(parser.parse('log("Hello, World!");', false));
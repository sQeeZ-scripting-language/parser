#include "parser/node/parser_node.hpp"

ParserNode::ParserNode(const Napi::CallbackInfo &args) : ObjectWrap(args) {
  this->_env = args.Env();
  if (args.Length() < 1 || !args[0].IsFunction()) {
    Napi::TypeError::New(this->_env, "Function expected").ThrowAsJavaScriptException();
  }
  Napi::Function require = args[0].As<Napi::Function>();
  std::regex self_regex("^function require\\(path\\)", std::regex_constants::ECMAScript | std::regex_constants::icase);
  if (!std::regex_search(require.ToString().Utf8Value().c_str(), self_regex)) {
    Napi::TypeError::New(this->_env, "{require} Function expected").ThrowAsJavaScriptException();
  }
  this->_require = Persistent(require);
}

Napi::String ParserNode::pingInstance(const Napi::CallbackInfo &args) {
  return Napi::String::New(this->_env, "Node instance of the sQeeZ-Parser is working!");
}

Napi::Array ParserNode::parse(const Napi::CallbackInfo &args) {
  if (args.Length() < 1 || !args[0].IsString()) {
    Napi::TypeError::New(this->_env, "Code expected!").ThrowAsJavaScriptException();
  }
  Napi::Env env = args.Env();
  auto code = args[0].As<Napi::String>().Utf8Value();
  bool devMode = false;
  if (args.Length() >= 2 && args[1].IsBoolean()) {
    devMode = args[1].As<Napi::Boolean>().Value();
  }
  Lexer lexer(code);
  std::vector<Token> tokens = lexer.tokenize(devMode);
  Parser parser(tokens);
  return programToJSArray(env, parser.parse(devMode));
}

Napi::Function ParserNode::GetClass(Napi::Env env) {
  return DefineClass(
      env, "ParserNode",
      {ParserNode::InstanceMethod("pingInstance", reinterpret_cast<InstanceMethodCallback>(&ParserNode::pingInstance)),
       ParserNode::InstanceMethod("parse", reinterpret_cast<InstanceMethodCallback>(&ParserNode::parse))});
}

Napi::String pingParser(const Napi::CallbackInfo &args) {
  Napi::Env env = args.Env();
  return Napi::String::New(env, "Node API for Parser is working!");
}

Napi::String info(const Napi::CallbackInfo &args) {
  Napi::Env env = args.Env();
  std::string info = R"(
    sQeeZ-Parser Node API Information:

    - Command Methods:
      1. info: Provides this API information.
      2. pingParser: Pings the module to check if it's responsive.

    - Instance Methods:
      1. ping: Pings the parser instance.
      2. parse: Processes and parses a provided code snippet.
    )";
  return Napi::String::New(env, info);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "pingParser"), Napi::Function::New(env, pingParser));
  exports.Set(Napi::String::New(env, "info"), Napi::Function::New(env, info));
  Napi::String name = Napi::String::New(env, "ParserNode");
  exports.Set(name, ParserNode::GetClass(env));
  return exports;
}

NODE_API_MODULE(parser, Init)
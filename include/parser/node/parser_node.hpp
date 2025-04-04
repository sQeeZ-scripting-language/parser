#ifndef PARSER_NODE_HPP
#define PARSER_NODE_HPP

#include <napi.h>

#include <iostream>
#include <regex>

#include "parser/parser.hpp"
#include "parser/node/node_parser.hpp"

Napi::String pingParser(const Napi::CallbackInfo &args);
Napi::String info(const Napi::CallbackInfo &args);

class ParserNode : public Napi::ObjectWrap<ParserNode> {
private:
  Napi::FunctionReference _require;
  Napi::Env _env = Napi::Env(nullptr);

public:
  explicit ParserNode(const Napi::CallbackInfo &args);

  Napi::String pingInstance(const Napi::CallbackInfo &args);
  Napi::Array parse(const Napi::CallbackInfo &args);

  static Napi::Function GetClass(Napi::Env);
};

#endif  // PARSER_NODE_HPP
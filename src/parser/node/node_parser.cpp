#include "parser/node/node_parser.hpp"

Napi::Array programToJSArray(const Napi::Env& env, const std::unique_ptr<Program>& program) {
  Napi::Array arr = Napi::Array::New(env, program->body.size());
  for (size_t i = 0; i < program->body.size(); ++i) {
    arr.Set(i, Napi::String::New(env, program->body[i]->toString()));
  }
  return arr;
}
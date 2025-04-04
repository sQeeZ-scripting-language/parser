#ifndef NODE_PARSER_HPP
#define NODE_PARSER_HPP

#include <napi.h>

#include "parser/parser.hpp"

Napi::Array programToJSArray(const Napi::Env& env, const std::unique_ptr<Program>& program);

#endif  // NODE_PARSER_HPP
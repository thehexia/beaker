#include "builtin.hpp"

namespace
{

std::unordered_map<std::string, Function_decl*> builtins_;

}


Function_decl*
bind_header()
{
  return nullptr;
}


Function_decl*
bind_field()
{
  return nullptr;
}

Function_decl*
alias_bind()
{
  return nullptr;
}


Function_decl*
advance()
{
  return nullptr;
}


Function_decl*
get_table()
{
  return nullptr;
}


Function_decl*
add_flow()
{
  return nullptr;
}

Function_decl*
match()
{
  return nullptr;
}


Function_decl*
load_field()
{
  return nullptr;
}



void init_builtins()
{
  builtins_ =
  {
    {"__bind_header", bind_header()},
    {"__bind_field", bind_field()},
    {"__alias_bind", alias_bind()},
    {"__advance", advance()},
    {"__get_table", get_table()},
    {"__add_flow", add_flow()},
    {"__match", match()},
    {"__load_field", load_field()},
  };
}

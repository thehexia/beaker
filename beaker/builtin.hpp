#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include "expr.hpp"
#include "stmt.hpp"
#include "decl.hpp"
#include "type.hpp"

// Define a set of global names for each builtin functions
// of flowpath functions
constexpr char const* __bind_header  = "fp_bind_header";
constexpr char const* __bind_field   = "fp_bind_field";
constexpr char const* __alias_bind   = "fp_alias_bind";
constexpr char const* __advance      = "fp_advance";
constexpr char const* __get_table    = "fp_get_table";
constexpr char const* __add_flow     = "fp_add_flow";
constexpr char const* __match        = "fp_goto_table";
constexpr char const* __load_field   = "fp_load_field";
constexpr char const* __get_port     = "fp_get_port";
constexpr char const* __gather       = "fp_gather";
constexpr char const* __output       = "fp_output2port";
constexpr char const* __drop         = "fp_drop";
constexpr char const* __context      = "__cxt__";
constexpr char const* __header       = "__header__";
constexpr char const* __table        = "__table__";
constexpr char const* __key          = "__key__";
constexpr char const* __drop_port    = "__drop";
constexpr char const* __flood_port   = "__flood";

// runtime interface functions
constexpr char const* __load         = "load";
constexpr char const* __process      = "process";
constexpr char const* __start        = "start";
constexpr char const* __stop         = "stop";
constexpr char const* __port_num     = "port_num";
constexpr char const* __unload       = "unload";


// Build all builtin functions
struct Builtin
{
  using Function_map = std::unordered_map<std::string, Function_decl*>;
  using Port_map = std::unordered_map<std::string, Port_decl*>;

  Builtin(Symbol_table& syms)
    : syms(syms)
  {
    init_builtins();
  }

  Function_decl* get_builtin_function(std::string name);

  Function_map get_builtin_functions() const { return builtin_fn; }
  Port_map     get_builtin_ports() const { return builtin_ports; }

  Expr* call_bind_field(Expr_seq const& args);
  Expr* call_bind_header();
  Expr* call_alias_field();
  Expr* call_advance(Expr_seq const& args);
  Expr* call_create_table(Decl*, Expr_seq const& args);
  Expr* call_add_flow(Expr_seq const& args);
  Expr* call_match(Expr_seq const& args);
  Expr* call_load_field(Expr_seq const& args);
  Expr* call_get_port(Decl*, Expr_seq const& args);
  Expr* call_gather(Expr* cxt, Expr_seq const& var_args);
  Expr* call_drop(Expr*);
  Expr* call_output(Expr_seq const&);

  // exposed interface
  Function_decl* load(Stmt_seq const&);
  Function_decl* unload();
  Function_decl* start();
  Function_decl* stop();
  Function_decl* port_num();

  // construct instrinsic flow functions
  // from flow declarations
  Function_decl* flow_fn(Symbol const*, Stmt*);

private:
  void init_builtins();

  Function_decl* bind_header();
  Function_decl* bind_field();
  Function_decl* alias_bind();
  Function_decl* advance();
  Function_decl* get_table();
  Function_decl* add_flow();
  Function_decl* gather();
  Function_decl* match();
  Function_decl* load_field();
  Function_decl* get_port();
  Function_decl* drop();
  Function_decl* output();

  Port_decl* drop_port();
  Port_decl* flood_port();

  Symbol const* get_identifier(std::string);

  Symbol_table& syms;
  Function_map builtin_fn;
  Port_map builtin_ports;
};


#endif

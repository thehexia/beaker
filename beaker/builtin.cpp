#include "builtin.hpp"
#include "builder.hpp"
#include "token.hpp"


// Helper function for constructing
// identifier symbols
Symbol const*
Builtin::get_identifier(std::string s)
{
  return syms.put<Identifier_sym>(s, identifier_tok);
}


//
// void __bind_header(int id, int length);
//
Function_decl*
Builtin::bind_header()
{
  Type const* void_type = get_void_type();
  Symbol const* fn_name = get_identifier(__bind_header);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("cxt"), get_reference_type(get_context_type())),
    new Parameter_decl(get_identifier("id"), get_integer_type()),
    new Parameter_decl(get_identifier("length"), get_integer_type()),
  };

  Type const* fn_type = get_function_type(parms, void_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


//
// void __bind_offset(Context*, id, offset, length);
//
Function_decl*
Builtin::bind_field()
{
  Type const* void_type = get_void_type();
  Symbol const* fn_name = get_identifier(__bind_field);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("cxt"), get_reference_type(get_context_type())),
    new Parameter_decl(get_identifier("id"), get_integer_type()),
    new Parameter_decl(get_identifier("offset"), get_integer_type()),
    new Parameter_decl(get_identifier("length"), get_integer_type()),
  };

  Type const* fn_type = get_function_type(parms, void_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


//
// void __alias_bind(Context*, int id1, int id2, int offset, int length);
//
Function_decl*
Builtin::alias_bind()
{
  Type const* void_type = get_void_type();
  Symbol const* fn_name = get_identifier(__alias_bind);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("cxt"), get_reference_type(get_context_type())),
    new Parameter_decl(get_identifier("id1"), get_integer_type()),
    new Parameter_decl(get_identifier("id2"), get_integer_type()),
    new Parameter_decl(get_identifier("offset"), get_integer_type()),
    new Parameter_decl(get_identifier("length"), get_integer_type()),
  };

  Type const* fn_type = get_function_type(parms, void_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


//
// void __advance(Context*, int n)
//
Function_decl*
Builtin::advance()
{
  Type const* void_type = get_void_type();
  Symbol const* fn_name = get_identifier(__advance);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("cxt"), get_reference_type(get_context_type())),
    new Parameter_decl(get_identifier("length"), get_integer_type()),
  };

  Type const* fn_type = get_function_type(parms, void_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


//
// Load_field loads a field into memory
// from a context.
//
// uint64_t load(Context*, int id)
//
// There may need to be an implicit truncation
// from 64 bits to the size of the field
Function_decl*
Builtin::load_field()
{
  // FIXME: once precision integers are added
  // make this a uint64_t instead (or the widest possible)
  // integer for the sake of safety.
  Type const* ret_type = get_integer_type();
  Symbol const* fn_name = get_identifier(__load_field);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("cxt"), get_reference_type(get_context_type())),
    new Parameter_decl(get_identifier("id"), get_integer_type()),
  };

  Type const* fn_type = get_function_type(parms, ret_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


Function_decl*
Builtin::get_table()
{
  // Table types are entirely opaque during code generation
  // so what the actual table type is doesnt matter as long
  // as it is a table type.
  Type const* ret_type = get_opaque_table();
  Symbol const* fn_name = get_identifier(__get_table);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("id"), get_integer_type()),
    new Parameter_decl(get_identifier("key_size"), get_integer_type()),
    new Parameter_decl(get_identifier("size"), get_integer_type()),
    new Parameter_decl(get_identifier("table_type"), get_integer_type())
  };

  Type const* fn_type = get_function_type(parms, ret_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


Function_decl*
Builtin::add_flow()
{
  // Table types are entirely opaque during code generation
  // so what the actual table type is doesnt matter as long
  // as it is a table type.
  Type const* tbl_ref = get_reference_type(get_table_type({}, {}));
  Type const* cxt_ref = get_reference_type(get_context_type());
  Type const* void_type = get_void_type();
  // Flows actually become free functions so they have function
  // type when lowered.
  Type_seq types {tbl_ref, cxt_ref};
  Type const* flow_fn_type = get_function_type(types, void_type);
  Type const* flow_type = get_reference_type(flow_fn_type);

  Symbol const* fn_name = get_identifier(__add_flow);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("table"), tbl_ref),
    new Parameter_decl(get_identifier("flow"), flow_type),
  };

  Type const* fn_type = get_function_type(parms, void_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}


Function_decl*
Builtin::match()
{
  // Table types are entirely opaque during code generation
  // so what the actual table type is doesnt matter as long
  // as it is a table type.
  Type const* ret_type = get_void_type();
  Type const* tbl_type = get_table_type({}, {});
  Symbol const* fn_name = get_identifier(__match);

  Decl_seq parms =
  {
    new Parameter_decl(get_identifier("context"), get_context_type()),
    new Parameter_decl(get_identifier("table"), tbl_type),
  };

  Type const* fn_type = get_function_type(parms, ret_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, parms, block({}));

  fn->declare_ = true;
  return fn;
}

Function_decl*
Builtin::get_port()
{
  Symbol const* fn_name = get_identifier(__get_port);

  Type const* port_type = get_port_type();

  Decl_seq parms;

  Type const* fn_type = get_function_type(parms, port_type);

  Function_decl* fn =
    new Function_decl(fn_name, fn_type, {}, block({}));

  fn->declare_ = true;
  return fn;
}


void
Builtin::init_builtins()
{
  builtins_ =
  {
    {__bind_header, bind_header()},
    {__bind_field, bind_field()},
    {__alias_bind, alias_bind()},
    {__advance, advance()},
    {__get_table, get_table()},
    {__add_flow, add_flow()},
    {__match, match()},
    // {__gather, gather()},
    {__load_field, load_field()},
    {__get_port, get_port()},
  };
}


Function_decl*
Builtin::load(Stmt_seq const& s)
{
  Type const* void_type = get_void_type();
  Symbol const* name = get_identifier(__load);

  Decl_seq parms;
  Type const* fn_type = get_function_type(parms, void_type);

  return new Function_decl(name, fn_type, parms, block(s));
}


Function_decl*
unload()
{
  throw std::runtime_error("unimplemented builtin");
}


Function_decl*
start()
{
  throw std::runtime_error("unimplemented builtin");
}


Function_decl*
stop()
{
  throw std::runtime_error("unimplemented builtin");
}


Function_decl*
port_num()
{
  throw std::runtime_error("unimplemented builtin");
}


Function_decl*
Builtin::flow_fn(Symbol const* name, Stmt* body)
{
  throw std::runtime_error("unimplemented builtin");
}



Function_decl*
Builtin::get_builtin_fn(std::string name)
{
  return builtins_.find(name)->second;
}


Expr*
Builtin::call_bind_field(Expr_seq const& args)
{
  Function_decl* fn = builtins_.find(__bind_field)->second;
  assert(fn);

  return new Bind_field(decl_id(fn), args);
}


Expr*
Builtin::call_load_field(Expr_seq const& args)
{
  Function_decl* fn = builtins_.find(__load_field)->second;
  assert(fn);

  return new Load_field(decl_id(fn), args);
}


Expr*
Builtin::call_create_table(Expr_seq const& args)
{
  Function_decl* fn = builtins_.find(__get_table)->second;
  assert(fn);

  return new Create_table(decl_id(fn), args);
}


Expr*
Builtin::call_advance(Expr_seq const& args)
{
  Function_decl* fn = builtins_.find(__advance) ->second;
  assert(fn);

  return new Advance(decl_id(fn), args);
}
// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "print.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "type.hpp"
#include <iostream>


// -------------------------------------------------------------------------- //
// Declarations

std::ostream& 
operator<<(std::ostream& os, Decl const& d)
{
  struct Fn
  {
    std::ostream& os;
    void operator()(Variable_decl const* d) { os << *d; };
    void operator()(Function_decl const* d) { os << *d; };
    void operator()(Parameter_decl const* d) { os << *d; };
    void operator()(Record_decl const* d) { os << *d; };
    void operator()(Field_decl const* d) { os << *d; };
    void operator()(Module_decl const* d) { os << *d; };

    // network declarations
    void operator()(Decode_decl const* d) { os << *d; };
    void operator()(Table_decl const* d) { os << *d; };
    void operator()(Flow_decl const* d) { os << *d; };
    void operator()(Port_decl const* d) { os << *d; };
    void operator()(Extracts_decl const* d) { os << *d; };
    void operator()(Rebind_decl const* d) { os << *d; };
  };

  apply(&d, Fn{os});
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Variable_decl const& d)
{
  return os << "var" << *d.name() << " : " << *d.type() << *d.init();
}


std::ostream& 
operator<<(std::ostream& os, Function_decl const& d)
{
  os << "def" << *d.name() << "(";
  Decl_seq const& parms = d.parameters();
  for (auto it = parms.begin(); it != parms.end(); ++it) {
    os << *it;
    if (it != parms.end() - 1)
      os << ", ";
  }
  os << ") ->";
  os << d.return_type() << d.body();

  return os;
}


std::ostream& 
operator<<(std::ostream& os, Parameter_decl const& d)
{
  return os << *d.name() << " : " << *d.type();
}


std::ostream& operator<<(std::ostream& os, Record_decl const& d)
{
  os << "record " << *d.name()
  << "\n{\n";

  for (auto decl : d.fields())
    os << *decl << '\n';

  os << "\n}\n";
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Field_decl const& d)
{
  return os << *d.name() << " : " << *d.type() << ";";
}


std::ostream& 
operator<<(std::ostream& os, Module_decl const& d)
{
  for (auto decl : d.declarations()) {
    os << *decl << '\n';
  }

  return os;
}


// network declarations
std::ostream& 
operator<<(std::ostream& os, Decode_decl const& d)
{
  if (d.is_start())
    os << "start ";

  os << "Decoder " << *d.name() << "(" << *d.header() << ")";

  return os;
}


std::ostream& 
operator<<(std::ostream& os, Table_decl const& d)
{
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Flow_decl const& d)
{
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Port_decl const& d)
{
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Extracts_decl const& d)
{
  return os;
}


std::ostream& 
operator<<(std::ostream& os, Rebind_decl const& d)
{
  return os;
}





// -------------------------------------------------------------------------- //
// Types


std::ostream& 
operator<<(std::ostream& os, Type const& t)
{
  struct Fn
  {
    std::ostream& os;

    void operator()(Id_type const* t) { os << *t; }
    void operator()(Boolean_type const* t) { os << *t; }
    void operator()(Integer_type const* t) { os << *t; }
    void operator()(Function_type const* t) { os << *t; }
    void operator()(Reference_type const* t) { os << *t; }
    void operator()(Record_type const* t) { os << *t; }
    void operator()(Void_type const* t) { os << *t; }

    // network specific types
    void operator()(Context_type const* t) { os << *t; }
    void operator()(Table_type const* t) { os << *t; }
    void operator()(Flow_type const* t) { os << *t; }
    void operator()(Port_type const* t) { os << *t; }
  };

  apply(&t, Fn{os});
  return os;
}


std::ostream&
operator<<(std::ostream& os, Id_type const& t)
{
  return os << "unresolved:" << *t.symbol();
}


std::ostream&
operator<<(std::ostream& os, Boolean_type const&)
{
  return os << "bool";
}


std::ostream&
operator<<(std::ostream& os, Integer_type const&)
{
  return os << "int";
}


std::ostream&
operator<<(std::ostream& os, Function_type const& t)
{
  os << '(';
  Type_seq const& parms = t.parameter_types();
  for (auto iter = parms.begin(); iter != parms.end(); ++iter) {
    os << **iter;
    if (std::next(iter) != parms.end())
      os << ',';
  }
  os << ')';
  os << " -> " << *t.return_type();
  return os;
}


// Just print the name of the type.
std::ostream&
operator<<(std::ostream& os, Record_type const& t)
{
  return os << *t.declaration()->name();
}


std::ostream&
operator<<(std::ostream& os, Void_type const&)
{
  return os << "void";
}


std::ostream&
operator<<(std::ostream& os, Context_type const&)
{
  return os << "Context";
}


// FIXME: print key fields correctly
std::ostream& 
operator<<(std::ostream& os, Table_type const& t) 
{
  // for (auto key : t->key_fields())
  //   os << key;

  return os << "table ";
}


std::ostream& 
operator<<(std::ostream& os, Flow_type const& t) 
{
  os << "flow(";
  for (auto key : t.key_types())
    os << key << " ";

  os << ")";

  return os;
}


std::ostream& 
operator<<(std::ostream& os, Port_type const& t) 
{
  return os << "port"; 
}


std::ostream&
operator<<(std::ostream& os, Reference_type const& t)
{
  return os << "ref " << *t.type();
}


// -------------------------------------------------------------------------- //
// Expressions

std::ostream&
operator<<(std::ostream& os, Expr const& e)
{
  struct Fn
  {
    std::ostream& os;

    void operator()(Literal_expr const* e) { os << *e; }
    void operator()(Id_expr const* e) { os << *e; }
    void operator()(Add_expr const* e) { os << *e; }
    void operator()(Sub_expr const* e) { os << *e; }
    void operator()(Mul_expr const* e) { os << *e; }
    void operator()(Div_expr const* e) { os << *e; }
    void operator()(Rem_expr const* e) { os << *e; }
    void operator()(Neg_expr const* e) { os << *e; }
    void operator()(Pos_expr const* e) { os << *e; }
    void operator()(Eq_expr const* e) { os << *e; }
    void operator()(Ne_expr const* e) { os << *e; }
    void operator()(Lt_expr const* e) { os << *e; }
    void operator()(Gt_expr const* e) { os << *e; }
    void operator()(Le_expr const* e) { os << *e; }
    void operator()(Ge_expr const* e) { os << *e; }
    void operator()(And_expr const* e) { os << *e; }
    void operator()(Or_expr const* e) { os << *e; }
    void operator()(Not_expr const* e) { os << *e; }
    void operator()(Call_expr const* e) { os << *e; }
    void operator()(Value_conv const* e) { os << *e; }
    void operator()(Default_init const* e) { os << *e; }
    void operator()(Copy_init const* e) { os << *e; }
  };
  apply(&e, Fn{os});
  return os;
}


std::ostream&
operator<<(std::ostream& os, Literal_expr const& e)
{
  return os << e.spelling();
}


std::ostream&
operator<<(std::ostream& os, Id_expr const& e)
{
  return os << e.spelling();
}


std::ostream&
operator<<(std::ostream& os, Add_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Sub_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Mul_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Div_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Rem_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Neg_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Pos_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Eq_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Ne_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Lt_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Gt_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Le_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Ge_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, And_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Or_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Not_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Call_expr const&)
{
  return os;
}


std::ostream&
operator<<(std::ostream& os, Value_conv const& e)
{
  return os << "__to_value("
            << *e.source() << ','
            << *e.target() << ')';
}


std::ostream&
operator<<(std::ostream& os, Default_init const& e)
{
  // return os;
  return os << "__default_init(" << *e.type() << ")";
}


std::ostream&
operator<<(std::ostream& os, Copy_init const& e)
{
  return os << "__copy_init(" << *e.type() << ',' << *e.value() << ")";
  // return os << *e.value();
}
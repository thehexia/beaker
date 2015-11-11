// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "print.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "type.hpp"
#include "stmt.hpp"
#include <iostream>



// -------------------------------------------------------------------------- //
// Stmts


std::ostream& 
operator<<(std::ostream& os, Stmt const& s)
{
  struct Fn
  {
    std::ostream& os;
    void operator()(Empty_stmt const* s) { os << *s; };
    void operator()(Block_stmt const* s) { os << *s; };
    void operator()(Assign_stmt const* s) { os << *s; };
    void operator()(Return_stmt const* s) { os << *s; };
    void operator()(If_then_stmt const* s) { os << *s; };
    void operator()(If_else_stmt const* s) { os << *s; };
    void operator()(Match_stmt const* s) { os << *s; };
    void operator()(Case_stmt const* s) { os << *s; };
    void operator()(While_stmt const* s) { os << *s; };
    void operator()(Break_stmt const* s) { os << *s; };
    void operator()(Continue_stmt const* s) { os << *s; };
    void operator()(Expression_stmt const* s) { os << *s; };
    void operator()(Declaration_stmt const* s) { os << *s; };
  };

  apply(&s, Fn{os});
  return os;
}


std::ostream& operator<<(std::ostream& os, Empty_stmt const& s)
{
  return os << ";";
}


std::ostream& operator<<(std::ostream& os, Block_stmt const& s)
{
  os << "\n{\n";
  for (auto stmt : s.statements()) {
    os << *stmt << '\n';
  }
  os << "\n}\n";

  return os;
}


std::ostream& operator<<(std::ostream& os, Assign_stmt const& s)
{
  return os << *s.object() << " = " << *s.value();
}


std::ostream& operator<<(std::ostream& os, Return_stmt const& s)
{
  return os << "return " << *s.value();
}


std::ostream& operator<<(std::ostream& os, If_then_stmt const& s)
{
  return os << "if(" << *s.condition() << ")" << *s.body();
}


std::ostream& operator<<(std::ostream& os, If_else_stmt const& s)
{
  return os << "if(" << *s.condition() << ")" << *s.true_branch() 
            << "else" << *s.false_branch();
}


std::ostream& operator<<(std::ostream& os, Match_stmt const& s)
{
  return os << "match";
}


std::ostream& operator<<(std::ostream& os, Case_stmt const& s)
{
  return os << "case";
}


std::ostream& operator<<(std::ostream& os, While_stmt const& s)
{
  return os << "while";
}


std::ostream& operator<<(std::ostream& os, Break_stmt const& s)
{
  return os << "break;";
}


std::ostream& operator<<(std::ostream& os, Continue_stmt const& s)
{
  return os << "continue;";
}


std::ostream& operator<<(std::ostream& os, Expression_stmt const& s)
{
  return os << *s.expression() << ';';
}


std::ostream& operator<<(std::ostream& os, Declaration_stmt const& s)
{
  return os << *s.declaration() << ';';
}




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

  os << "Decoder " << *d.name() << "(" << *d.header() << ")" << *d.body();

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
    void operator()(Character_type const* t) { os << *t; }
    void operator()(Integer_type const* t) { os << *t; }
    void operator()(Function_type const* t) { os << *t; }
    void operator()(Block_type const* t) { os << *t; }
    void operator()(Array_type const* t) { os << *t; }
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
operator<<(std::ostream& os, Character_type const&)
{
  return os << "char";
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
operator<<(std::ostream& os, Array_type const& t)
{
  return os << *t.type() << '[' << *t.extent() << ']';
}


std::ostream&
operator<<(std::ostream& os, Block_type const& t)
{
  return os << *t.type() << "[]";
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
    void operator()(Member_expr const* e) { os << *e; }
    void operator()(Index_expr const* e) { os << *e; }
    void operator()(Value_conv const* e) { os << *e; }
    void operator()(Block_conv const* e) { os << *e; }
    void operator()(Default_init const* e) { os << *e; }
    void operator()(Copy_init const* e) { os << *e; }
    void operator()(Dot_expr const* e) { os << *e; }
    void operator()(Field_name_expr const* e) { os << *e; }
  };
  apply(&e, Fn{os});
  return os;
}


std::ostream&
operator<<(std::ostream& os, Literal_expr const& e)
{
  return os << e.value();
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
operator<<(std::ostream& os, Member_expr const& e)
{
  return os << *e.scope() << '.' << *e.member();
}


std::ostream&
operator<<(std::ostream& os, Index_expr const& e)
{
  return os << *e.array() << '[' << *e.index() << ']';
}


std::ostream&
operator<<(std::ostream& os, Value_conv const& e)
{
  return os << "__to_value("
            << *e.source() << ','
            << *e.target() << ')';
}


std::ostream&
operator<<(std::ostream& os, Block_conv const& e)
{
  return os << "__to_block("
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


std::ostream&
operator<<(std::ostream& os, Dot_expr const& e)
{
  return os << *e.object() << "." << *e.member();
}


std::ostream& operator<<(std::ostream& os, Field_name_expr const& e)
{
  Expr_seq const& ids = e.identifiers();

  for (auto it = ids.begin(); it != ids.end(); ++it) {
    os << **it;
    if (it != ids.end() - 1)
      os << "::";
  }
  return os;
}

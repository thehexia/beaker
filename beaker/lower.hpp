#ifndef LOWER_HPP
#define LOWER_HPP

#include "builder.hpp"
#include "builtin.hpp"
#include "type.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "elaborator.hpp"
#include "pipeline.hpp"
#include "length.hpp"
#include "offset.hpp"

struct Lowerer
{
  struct Scope_sentinel;

  Lowerer(Elaborator& elab, Pipeline_checker checker)
    : elab(elab), builtin(elab.syms), checker(checker)
  { }

  Expr* lower(Expr*);
  Expr* lower(Field_access_expr* e);

  Decl* lower_global_decl(Decl*);
  Decl* lower_global_decl(Decode_decl*);
  Decl* lower_global_decl(Table_decl*);
  Decl* lower_global_decl(Port_decl*);

  Decl* lower_global_def(Decl*);
  Decl* lower_global_def(Decode_decl*);
  Decl* lower_global_def(Table_decl*);
  Decl* lower_global_def(Port_decl*);

  Decl* lower(Decl*);
  Decl* lower(Module_decl*);

  // network declarations
  Decl* lower(Layout_decl*);
  Decl* lower(Decode_decl*);
  Decl* lower(Table_decl*);
  Decl* lower(Key_decl*);
  Decl* lower(Flow_decl*);
  Decl* lower(Port_decl*);

  Stmt_seq lower_extracts_decl(Extracts_decl*);
  Stmt_seq lower_rebind_decl(Rebind_decl*);

  Stmt_seq lower(Stmt*);
  Stmt_seq lower(Block_stmt*);
  Stmt_seq lower(If_then_stmt*);
  Stmt_seq lower(If_else_stmt*);
  Stmt_seq lower(Match_stmt*);
  Stmt_seq lower(Case_stmt*);
  Stmt_seq lower(While_stmt*);
  Stmt_seq lower(Expression_stmt*);
  Stmt_seq lower(Declaration_stmt*);
  Stmt_seq lower(Decode_stmt*);
  Stmt_seq lower(Goto_stmt*);

  void declare(Decl*);
  void redeclare(Decl*);
  void overload(Overload&, Decl*);
  Symbol const* get_identifier(std::string);
  Overload* unqualified_lookup(Symbol const*);
  Overload* qualified_lookup(Scope*, Symbol const*);

  Elaborator& elab;
  Scope_stack stack;
  Builtin builtin;
  Pipeline_checker checker;

private:
};


struct Lowerer::Scope_sentinel
{
  Scope_sentinel(Lowerer& e, Decl* d = nullptr)
    : lower(e)
  {
    lower.stack.push(d);
  }

  ~Scope_sentinel()
  {
    lower.stack.pop();
  }

  Lowerer& lower;
};

#endif

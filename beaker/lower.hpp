#ifndef LOWER_HPP
#define LOWER_HPP

#include "builder.hpp"
#include "builtin.hpp"
#include "type.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "elaborator.hpp"


struct Lowerer
{
  struct Scope_sentinel;

  Lowerer(Elaborator& elab)
    : elab(elab), builtin(elab.syms)
  { }

  Expr* lower(Expr*);
  // Expr* lower(Literal_expr*);
  // Expr* lower(Id_expr*);
  // Expr* lower(Decl_expr*);
  // Expr* lower(Add_expr* e);
  // Expr* lower(Sub_expr* e);
  // Expr* lower(Mul_expr* e);
  // Expr* lower(Div_expr* e);
  // Expr* lower(Rem_expr* e);
  // Expr* lower(Neg_expr* e);
  // Expr* lower(Pos_expr* e);
  // Expr* lower(Eq_expr* e);
  // Expr* lower(Ne_expr* e);
  // Expr* lower(Lt_expr* e);
  // Expr* lower(Gt_expr* e);
  // Expr* lower(Le_expr* e);
  // Expr* lower(Ge_expr* e);
  // Expr* lower(And_expr* e);
  // Expr* lower(Or_expr* e);
  // Expr* lower(Not_expr* e);
  // Expr* lower(Call_expr* e);
  // Expr* lower(Dot_expr* e);
  // Expr* lower(Field_expr* e);
  // Expr* lower(Method_expr* e);
  // Expr* lower(Index_expr* e);
  // Expr* lower(Value_conv* e);
  // Expr* lower(Block_conv* e);
  // Expr* lower(Default_init* e);
  // Expr* lower(Copy_init* e);
  // Expr* lower(Reference_init* e);

  Expr* lower(Field_name_expr* e);

  Decl* lower(Decl*);
  // Decl* lower(Variable_decl*);
  // Decl* lower(Function_decl*);
  // Decl* lower(Parameter_decl*);
  // Decl* lower(Record_decl*);
  // Decl* lower(Field_decl*);
  // Decl* lower(Method_decl*);
  Decl* lower(Module_decl*);

  // network declarations
  Decl* lower(Layout_decl*);
  Decl* lower(Decode_decl*);
  Decl* lower(Table_decl*);
  Decl* lower(Key_decl*);
  Decl* lower(Flow_decl*);
  Decl* lower(Port_decl*);
  Decl* lower(Extracts_decl*);
  Decl* lower(Rebind_decl*);

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
  Symbol const* get_identifier(char const*);
  Overload* unqualified_lookup(Symbol const*);
  Overload* qualified_lookup(Scope*, Symbol const*);

  Elaborator& elab;
  Scope_stack stack;
  Builtin builtin;

private:

  struct Lower_decl_stmt;
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

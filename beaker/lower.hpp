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
  Lowerer(Elaborator& elab)
    : elab(elab), stack(stack)
  { }

  Stmt_seq lower(Stmt const*);
  Expr* lower(Expr const*);
  Decl* lower(Decl const*);

  Elaborator& elab;
  Scope_stack& stack;
};


#endif

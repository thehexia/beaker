#ifndef BUILDER_HPP
#define BUILER_HPP

#include "token.hpp"
#include "expr.hpp"


// Boolean building

// Literal zero
inline Expr*
zero()
{
  static Integer_sym sym(integer_tok, 0);
  return new Literal_expr(&sym);
}


// Literal one
inline Expr*
one()
{
  static Integer_sym sym(integer_tok, 1);
  return new Literal_expr(&sym);
}


// Make an arbitrary integer literal
inline Expr*
make_int(int n)
{
  Integer_sym* int_sym = new Integer_sym(integer_tok, n);
  return new Literal_expr(int_sym);
}


// ----------------------------------------------------- //
//      Expression building

// Add
inline Expr*
add(Expr* a, Expr* b)
{
  return new Add_expr(a, b);
}


// subtract
inline Expr*
sub(Expr* a, Expr* b)
{
  return new Sub_expr(a, b);
}


// divide
inline Expr*
div(Expr* a, Expr* b)
{
  return new Div_expr(a, b);
}


// multiply
inline Expr*
mul(Expr* a, Expr* b)
{
  return new Mul_expr(a, b);
}


// ----------------------------------------------------- //
//      Function building




#endif
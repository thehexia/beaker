#ifndef BUILDER_HPP
#define BUILER_HPP

#include "token.hpp"
#include "expr.hpp"
#include "type.hpp"


// Boolean building

// Literal zero
inline Expr*
zero()
{
  return new Literal_expr(get_integer_type(), 0);
}


// Literal one
inline Expr*
one()
{
  return new Literal_expr(get_integer_type(), 1);
}


// Make an arbitrary integer literal
inline Expr*
make_int(int n)
{
  return new Literal_expr(get_integer_type(), n);
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
#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "expr.hpp"
#include "type.hpp"
#include "stmt.hpp"


// ------------------------------------------------ //
//      Required Instructions


struct Instruction : Stmt
{
  void accept(Visitor& v) const { return v.visit(this); }
  void accept(Mutator& v)       { return v.visit(this); }
};


struct Drop : Instruction
{
  void accept(Visitor& v) const { return v.visit(this); }
  void accept(Mutator& v)       { return v.visit(this); }
};


struct Apply_add_field : Instruction
{
};


struct Apply_rmv_field : Instruction
{
};


struct Apply_get_field : Instruction
{
};


// Output the packet to a given
// port.
struct Output : Instruction
{
  Output(Expr* e)
    : port_(e)
  { }

  Expr* port() const { return port_; }

  void accept(Visitor& v) const { return v.visit(this); }
  void accept(Mutator& v)       { return v.visit(this); }

  Expr* port_;
};


// Goto a group table
struct Apply_group : Instruction
{
};


struct Apply_set_field : Instruction
{
};

// Write an output to port acttion
// to the context
struct Write_output : Instruction
{
};

// Write set field
struct Write_set_field : Instruction
{
};


// Write add field
struct Write_add_field : Instruction
{
};


// Write rmv field
struct Write_rmv_field : Instruction
{
};


#endif

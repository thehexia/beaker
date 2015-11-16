// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef BEAKER_ELABORATOR_HPP
#define BEAKER_ELABORATOR_HPP

#include "prelude.hpp"
#include "location.hpp"
#include "overload.hpp"
#include "environment.hpp"
#include "overload.hpp"
// #include "pipeline.hpp"

// The elaborator is responsible for a number of static
// analyses. In particular, it resolves identifiers and
// types expressions.
//
// FIXME: The type checking here is fundamentally broken.
// Instead of throwing exceptions, we should be documenting
// errors and continuing elaboration. There may be some
// cases where elaboration must stop.

#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>


// A scope defines a maximal lexical region of a program
// where no bindings are destroyed. A scope optionally
// assocaites a declaration with its bindings. This is
// used to maintain the current declaration context.
struct Scope : Environment<Symbol const*, Overload>
{
  Scope() 
    : decl(nullptr)
  { }

  Scope(Decl* d)
    : decl(d)
  { }

  Overload& bind(Symbol const*, Decl*);

  Decl* decl;
};


// The scope stack maintains the current scope during
// elaboration. It adapts the more general stack to
// provide more language-specific names for those
// operations.
struct Scope_stack : Stack<Scope>
{
  Scope&       current()       { return top(); }
  Scope const& current() const { return top(); }

  Scope&       global()       { return bottom(); }
  Scope const& global() const { return bottom(); }

  Decl*          context() const;
  Module_decl*   module() const;
  Function_decl* function() const;

  void declare(Decl*);
};


// Maintains a sequence of declarations
// which comprise a pipeline and retain
// the module in which the pipeline resides
struct Pipeline_decls : Decl_seq
{
  Pipeline_decls(Module_decl* m)
    : Decl_seq(), module_(m)
  { }

  Module_decl const* module() const { return module_; }

  Module_decl* module_;
};


struct Pipeline_stack : std::vector<Pipeline_decls>
{
  void insert(Decl* d) { this->back().push_back(d); }
  void new_pipeline(Module_decl* m) { this->push_back(Pipeline_decls(m)); }
};


// The elaborator is responsible for the annotation of
// an AST with type and other information.
class Elaborator
{
  struct Scope_sentinel;

  // friend class Pipeline_checker;

public:
  Elaborator(Location_map&);

  Type const* elaborate(Type const*);
  Type const* elaborate(Id_type const*);
  Type const* elaborate(Boolean_type const*);
  Type const* elaborate(Character_type const*);
  Type const* elaborate(Integer_type const*);
  Type const* elaborate(Function_type const*);
  Type const* elaborate(Array_type const*);
  Type const* elaborate(Block_type const*);
  Type const* elaborate(Reference_type const*);
  Type const* elaborate(Record_type const*);
  Type const* elaborate(Void_type const*);

  // network specific types
  Type const* elaborate(Layout_type const*);
  Type const* elaborate(Context_type const*);
  Type const* elaborate(Table_type const*);
  Type const* elaborate(Flow_type const*);
  Type const* elaborate(Port_type const*);

  Expr* elaborate(Expr*);
  Expr* elaborate(Literal_expr*);
  Expr* elaborate(Id_expr*);
  Expr* elaborate(Add_expr* e);
  Expr* elaborate(Sub_expr* e);
  Expr* elaborate(Mul_expr* e);
  Expr* elaborate(Div_expr* e);
  Expr* elaborate(Rem_expr* e);
  Expr* elaborate(Neg_expr* e);
  Expr* elaborate(Pos_expr* e);
  Expr* elaborate(Eq_expr* e);
  Expr* elaborate(Ne_expr* e);
  Expr* elaborate(Lt_expr* e);
  Expr* elaborate(Gt_expr* e);
  Expr* elaborate(Le_expr* e);
  Expr* elaborate(Ge_expr* e);
  Expr* elaborate(And_expr* e);
  Expr* elaborate(Or_expr* e);
  Expr* elaborate(Not_expr* e);
  Expr* elaborate(Call_expr* e);
  Expr* elaborate(Member_expr* e);
  Expr* elaborate(Index_expr* e);
  Expr* elaborate(Value_conv* e);
  Expr* elaborate(Block_conv* e);
  Expr* elaborate(Default_init* e);
  Expr* elaborate(Copy_init* e);
  Expr* elaborate(Dot_expr* e);
  Expr* elaborate(Field_name_expr* e);

  Decl* elaborate(Decl*);
  Decl* elaborate(Variable_decl*);
  Decl* elaborate(Function_decl*);
  Decl* elaborate(Parameter_decl*);
  Decl* elaborate(Record_decl*);
  Decl* elaborate(Field_decl*);
  Decl* elaborate(Module_decl*);

  // network declarations
  Decl* elaborate(Layout_decl*);
  Decl* elaborate(Decode_decl*);
  Decl* elaborate(Table_decl*);
  Decl* elaborate(Flow_decl*);
  Decl* elaborate(Port_decl*);
  Decl* elaborate(Extracts_decl*);
  Decl* elaborate(Rebind_decl*);

  // FIXME: Is there any real reason that these return
  // types? What is the type of an if statement?
  Stmt* elaborate(Stmt*);
  Stmt* elaborate(Empty_stmt*);
  Stmt* elaborate(Block_stmt*);
  Stmt* elaborate(Assign_stmt*);
  Stmt* elaborate(Return_stmt*);
  Stmt* elaborate(If_then_stmt*);
  Stmt* elaborate(If_else_stmt*);
  Stmt* elaborate(Match_stmt*);
  Stmt* elaborate(Case_stmt*);
  Stmt* elaborate(While_stmt*);
  Stmt* elaborate(Break_stmt*);
  Stmt* elaborate(Continue_stmt*);
  Stmt* elaborate(Expression_stmt*);
  Stmt* elaborate(Declaration_stmt*);
  Stmt* elaborate(Decode_stmt*);
  Stmt* elaborate(Goto_stmt*);

  // Found symbols.
  Function_decl* main = nullptr;

private:
  Location_map locs;
  Scope_stack  stack;

  // maintain the set of declarations which have been
  // forward declared
  std::unordered_set<Decl*> fwd_set;

  // maintain a list of pipeline decls per module
  Pipeline_stack pipelines; 

  void forward_declare(Decl_seq const&);
};


inline
Elaborator::Elaborator(Location_map& loc)
  : locs(loc)
{ }


struct Elaborator::Scope_sentinel
{
  Scope_sentinel(Elaborator& e, Decl* d = nullptr)
    : elab(e)
  {
    elab.stack.push(d);
  }

  ~Scope_sentinel()
  {
    elab.stack.pop();
  }

  Elaborator& elab;
};


#endif

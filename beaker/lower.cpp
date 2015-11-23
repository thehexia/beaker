#include "lower.hpp"
#include "error.hpp"


namespace
{

struct Lower_expr_fn
{
  Lowerer& lower;

  // catch all case
  // simply return the original
  // expression without lowering it
  template<typename T>
  Expr* operator()(T* e) const { return e; }

  // Field name expr
  // becomes an id_expr whose declaration is
  // resolved against a variable created by lowering
  // the extracts decl
  Expr* operator()(Field_name_expr* e) const { return lower.lower(e); }
};


struct Lower_decl_fn
{
  Lowerer& lower;

  // catch all case
  // return the original declaration
  template<typename T>
  Decl* operator()(T* d) { return d; }

  Decl* operator()(Module_decl* d) { return lower.lower(d); }

  // network declarations
  Decl* operator()(Decode_decl* d) { return lower.lower(d); }
  Decl* operator()(Table_decl* d) { return lower.lower(d); }
  Decl* operator()(Flow_decl* d) { return lower.lower(d); }
  Decl* operator()(Port_decl* d) { return lower.lower(d); }
};


struct Lower_stmt_fn
{
  Lowerer& lower;

  // catch all case
  template<typename T>
  Stmt_seq operator()(T* s) { return { s }; }

  Stmt_seq operator()(Empty_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Block_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Assign_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(If_then_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(If_else_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Match_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Case_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(While_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Expression_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Declaration_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Decode_stmt* s) { return lower.lower(s); }
  Stmt_seq operator()(Goto_stmt* s) { return lower.lower(s); }
};

} // namespace


// ------------------------------------------------------------------------- //
//                    Lower Expressions


Expr*
Lowerer::lower(Expr* e)
{
  return apply(e, Lower_expr_fn{*this});
}


Expr*
Lowerer::lower(Field_name_expr* e)
{
  return e;
}


// ------------------------------------------------------------------------- //
//                    Lower Declarations

Decl*
Lowerer::lower(Decl* d)
{
  return apply(d, Lower_decl_fn{*this});
}


Decl*
Lowerer::lower(Module_decl* d)
{
  Scope_sentinel scope(*this, d);

  Decl_seq module_decls;

  for (Decl* decl : d->declarations()) {
    declare(decl);
  }

  for (Decl* decl : d->declarations()) {
    module_decls.push_back(lower(decl));
  }

  return d;
}


Decl*
Lowerer::lower(Decode_decl* d)
{

  return d;
}


Decl*
Lowerer::lower(Table_decl* d)
{

  return d;
}


Decl*
Lowerer::lower(Flow_decl* d)
{

  return d;
}


Decl*
Lowerer::lower(Port_decl* d)
{

  return d;
}


// -------------------------------------------------------------------------- //
//                    Lowering Statements



Stmt_seq
Lowerer::lower(Stmt* s)
{
  return apply(s, Lower_stmt_fn{*this});
}


Stmt_seq
Lowerer::lower(Empty_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Block_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Assign_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Return_stmt* s)
{
}


Stmt_seq
Lowerer::lower(If_then_stmt* s)
{
}


Stmt_seq
Lowerer::lower(If_else_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Match_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Case_stmt* s)
{
}


Stmt_seq
Lowerer::lower(While_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Expression_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Declaration_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Decode_stmt* s)
{
}


Stmt_seq
Lowerer::lower(Goto_stmt* s)
{
}


// -------------------------------------------------------------------------- //
// Declaration of entities


// Determine if d can be overloaded with the existing
// elements in the set.
void
Lowerer::overload(Overload& ovl, Decl* curr)
{
  // Check to make sure that curr does not conflict with any
  // declarations in the current overload set.
  for (Decl* prev : ovl) {
    // If the two declarations have the same type, this
    // is not overloading. It is redefinition.
    if (prev->type() == curr->type()) {
      std::stringstream ss;
      ss << "redefinition of " << *curr->name() << '\n';
      throw Type_error({}, ss.str());
    }

    if (!can_overload(prev, curr)) {
      std::stringstream ss;
      ss << "cannot overload " << *curr->name() << '\n';
      throw Type_error({}, ss.str());
    }
  }

  ovl.push_back(curr);
}


// Create a declarative binding for d. This also checks
// that the we are not redefining a symbol in the current
// scope.
void
Lowerer::declare(Decl* d)
{
  Scope& scope = stack.current();

  // Set d's declaration context.
  d->cxt_ = stack.context();

  // If we've already seen the name, we should
  // determine if it can be overloaded.
  if (Scope::Binding* bind = scope.lookup(d->name()))
    return overload(bind->second, d);

  // Create a new overload set.
  Scope::Binding& bind = scope.bind(d->name(), {});
  Overload& ovl = bind.second;
  ovl.push_back(d);
}


// When opening the scope of a previously declared
// entity, simply push the declaration into its
// overload set.
void
Lowerer::redeclare(Decl* d)
{
  Scope& scope = stack.current();
  Overload* ovl;
  if (Scope::Binding* bind = scope.lookup(d->name()))
    ovl = &bind->second;
  else
    ovl = &scope.bind(d->name(), {}).second;
  ovl->push_back(d);
}


// Perform lookup of an unqualified identifier. This
// will search enclosing scopes for the innermost
// binding of the identifier.
Overload*
Lowerer::unqualified_lookup(Symbol const* sym)
{
  if (Scope::Binding* bind = stack.lookup(sym))
    return &bind->second;
  else
    return nullptr;
}


// Perform a qualified lookup of a name in the given
// scope. This searches only that scope for a binding
// for the identifier.
Overload*
Lowerer::qualified_lookup(Scope* s, Symbol const* sym)
{
  if (Scope::Binding* bind = s->lookup(sym))
    return &bind->second;
  else
    return nullptr;
}

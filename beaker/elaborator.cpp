// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "elaborator.hpp"
#include "type.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "stmt.hpp"
#include "convert.hpp"
#include "evaluator.hpp"
#include "error.hpp"

#include <iostream>
#include <set>


// -------------------------------------------------------------------------- //
// Lexical scoping


Overload& 
Scope::bind(Symbol const* sym, Decl* d)
{
  Overload ovl { d };
  Binding& ins = Environment::bind(sym, ovl);
  return ins.second;
}


// Create a declarative binding for d. This also checks
// that the we are not redefining a symbol in the current
// scope.
void
Scope_stack::declare(Decl* d)
{
  Scope& scope = current();

  if (auto binding = scope.lookup(d->name())) {
    // check to see if overloading is possible
    // the second member of the pair is an overload set
    // if it is not possible this call will produce an error
    if (overload_decl(&binding->second, d)) {
      // set the declaration context
      d->cxt_ = context();
      return;
    }

    // TODO: Add a note that points to the previous definition
    std::stringstream ss;
    ss << "redefinition of '" << *d->name() << "'\n";
    throw Lookup_error({}, ss.str());
    return;
  }

  // Create the binding.
  scope.bind(d->name(), d);

  // Set d's declaration context.
  d->cxt_ = context();
}


// Returns the innermost declaration context.
Decl*
Scope_stack::context() const
{
  for (auto iter = rbegin(); iter != rend(); ++iter) {
    Scope const& s = *iter;
    if (s.decl)
      return s.decl;
  }
  return nullptr;
}


// Returns the current module. This always the bottom
// of the stack.
Module_decl*
Scope_stack::module() const
{
  return cast<Module_decl>(bottom().decl);
}


// Returns the current function. The current function is found
// by working outwards through the declaration context stack.
// If there is no current function, this returns nullptr.
Function_decl*
Scope_stack::function() const
{
  for (auto iter = rbegin(); iter != rend(); ++iter) {
    Scope const& s = *iter;
    if (Function_decl* fn = as<Function_decl>(s.decl))
      return fn;
  }
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Elaboration of types

// Handling for forward declaring all top module-level
// declarations
void 
Elaborator::forward_declare(Decl_seq const& seq)
{
  for (auto it = seq.begin(); it < seq.end(); ++it) {
    fwd_set.insert(*it);
    stack.declare(*it);
  }
}



Type const*
Elaborator::elaborate(Type const* t)
{
  struct Fn
  {
    Elaborator& elab;

    Type const* operator()(Id_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Boolean_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Character_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Integer_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Function_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Block_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Array_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Reference_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Record_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Void_type const* t) { return elab.elaborate(t); }

    Type const* operator()(Layout_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Context_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Table_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Flow_type const* t) { return elab.elaborate(t); }
    Type const* operator()(Port_type const* t) { return elab.elaborate(t); }
  };
  return apply(t, Fn{*this});
}


Type const*
Elaborator::elaborate(Id_type const* t)
{
  Scope::Binding const* b = stack.lookup(t->symbol());
  if (!b) {
    std::stringstream ss;
    ss << "no matching declaration for '" << *t->symbol() << '\'';
    throw Lookup_error(locs.get(t), ss.str());
  }

  // Determine if the name is a type declaration.
  Decl* d = b->second.front();
  if (Record_decl* r = as<Record_decl>(d)) {
    return get_record_type(r);
  }
  else if (Layout_decl* l = as<Layout_decl>(d)) {
    return get_layout_type(l); 
  }
  else {
    std::stringstream ss;
    ss << '\'' << *t->symbol() << "' does not name a type";
    throw Lookup_error(locs.get(t), ss.str());
  }
}


Type const*
Elaborator::elaborate(Boolean_type const* t)
{
  return t;
}


Type const*
Elaborator::elaborate(Character_type const* t)
{
  return t;
}


Type const*
Elaborator::elaborate(Integer_type const* t)
{
  return t;
}


// Elaborate each type in the function type.
Type const*
Elaborator::elaborate(Function_type const* t)
{
  Type_seq ts;
  ts.reserve(t->parameter_types().size());
  for (Type const* t1 : t->parameter_types())
    ts.push_back(elaborate(t1));
  Type const* r = elaborate(t->return_type());
  return get_function_type(ts, r);
}


Type const*
Elaborator::elaborate(Array_type const* t)
{
  Type const* t1 = elaborate(t->type());
  Expr* e = elaborate(t->extent());
  Expr* n = reduce(e);
  if (!n)
    throw Type_error({}, "non-constant array extent");
  return get_array_type(t1, n);
}

Type const*
Elaborator::elaborate(Block_type const* t)
{
  Type const* t1 = elaborate(t->type());
  return get_block_type(t1);
}


Type const*
Elaborator::elaborate(Reference_type const* t)
{
  Type const* t1 = elaborate(t->type());
  return get_reference_type(t1);
}


// No further elaboration is needed.
Type const*
Elaborator::elaborate(Record_type const* t)
{
  return t;
}


Type const*
Elaborator::elaborate(Void_type const* t)
{
  return t;
}


// No further elaboration is needed.
Type const*
Elaborator::elaborate(Layout_type const* t)
{
  return t;
}


Type const* 
Elaborator::elaborate(Context_type const* t)
{
  return t;
}

// no further elaboration required
Type const* 
Elaborator::elaborate(Table_type const* t)
{
  return t;
}

// no further elaboration required
Type const* 
Elaborator::elaborate(Flow_type const* t)
{
  return t;
}


// No further elaboration required
Type const* 
Elaborator::elaborate(Port_type const* t)
{
  return t;
}


// -------------------------------------------------------------------------- //
// Elaboration of expressions

// Returns the type of an expression. This also annotates
// the expression by saving the computed type as part of
// the expression.
Expr*
Elaborator::elaborate(Expr* e)
{
  struct Fn
  {
    Elaborator& elab;

    Expr* operator()(Literal_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Id_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Add_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Sub_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Mul_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Div_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Rem_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Neg_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Pos_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Eq_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Ne_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Lt_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Gt_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Le_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Ge_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(And_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Or_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Not_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Call_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Member_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Index_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Value_conv* e) const { return elab.elaborate(e); }
    Expr* operator()(Block_conv* e) const { return elab.elaborate(e); }
    Expr* operator()(Default_init* e) const { return elab.elaborate(e); }
    Expr* operator()(Copy_init* e) const { return elab.elaborate(e); }
    Expr* operator()(Dot_expr* e) const { return elab.elaborate(e); }
    Expr* operator()(Field_name_expr* e) const { return elab.elaborate(e); }
  };

  return apply(e, Fn{*this});
}


// Literal expressions are fully elaborated at the point
// of construction.
Expr*
Elaborator::elaborate(Literal_expr* e)
{
  return e;
}


// Elaborate an id expression. When the identifier refers
// to an object of type T (a variable or parameter), the
// type of the expression is T&. Otherwise, the type of the
// expression is the type of the declaration.
//
// TODO: There may be some contexts in which an unresolved
// identifier can be useful. Unfortunately, this means that
// we have to push the handling of lookup errors up one
// layer, unless we to precisely establish contexts where
// such identifiers are allowed.
//
// TODO: If the lookup is resolved, should we actually 
// return a different kind of expression?
Expr*
Elaborator::elaborate(Id_expr* e)
{
  Scope::Binding const* b = stack.lookup(e->symbol());
  if (!b) {
    std::stringstream ss;
    ss << "no matching declaration for '" << *e->symbol() << '\'';
    throw Lookup_error(locs.get(e), ss.str());
  }

  // Annotate the expression with its declaration.
  Decl* d = b->second.front();
  e->declaration(d);

  // If the referenced declaration is a variable of
  // type T, then the type is T&. Otherwise, it is 
  // just T.
  Type const* t = d->type();
  if (defines_object(d))
    t = t->ref();
  e->type_ = t;

  return e;
}


namespace
{

// Used to require the conversion of a reference to a
// value. Essentially, this unwraps the reference if
// needed.
Expr*
require_value(Elaborator& elab, Expr* e)
{
  e = elab.elaborate(e);
  e = convert_to_value(e);
  return e;
}


// Used to require the conversion of an expression
// to a given type. This returns nullptr if the convesion 
// fails.
Expr*
require_converted(Elaborator& elab, Expr* e, Type const* t)
{
  e = elab.elaborate(e);
  e = convert(e, t);
  return e;
}


// The operands of a binary arithmetic expression are
// converted to rvalues. The converted operands shall have
// type int. The result of an arithmetic expression is an
// rvalue with type int.
template<typename T>
Expr*
check_binary_arithmetic_expr(Elaborator& elab, T* e)
{
  Type const* z = get_integer_type();
  Expr* c1 = require_converted(elab, e->first, z);
  Expr* c2 = require_converted(elab, e->second, z);
  if (!c1)
    throw Type_error({}, "left operand cannot be converted to 'int'");
  if (!c2)
    throw Type_error({}, "right operand cannot be converted to 'int'");

  // Rebuild the expression with the
  // converted operands.
  e->type_ = z;
  e->first = c1;
  e->second = c2;
  return e;
}


// The operands of a unary arithmetic expression are
// converted to rvalues. The converted operands shall
// have type int. The result of an arithmetic expression
// is an rvalue of type int.
template<typename T>
Expr*
check_unary_arithmetic_expr(Elaborator& elab, T* e)
{
  // Apply conversions
  Type const* z = get_integer_type();
  Expr* c = require_converted(elab, e->first, z);
  if (!c)
    throw Type_error({}, "operand cannot be converted to 'int'");

  // Rebuild the expression with the converted operands.
  e->type_ = z;
  e->first = c;
  return e;
}


// Check all flows within a table initializer
// to confirm that the keys given are of the
// correct type.
bool
check_table_initializer(Elaborator& elab, Table_decl* d)
{
  assert(is<Table_type>(d->type()));

  Table_type const* table_type = as<Table_type>(d->type());

  Type_seq const& field_types = table_type->field_types();

  // key track of the flow in the initializer
  int flow_cnt = 0;

  for (auto f : d->body()) {
    assert(is<Flow_decl>(f));

    ++flow_cnt;
    Flow_decl* flow = as<Flow_decl>(f);
    Expr_seq const& key = flow->keys();

    // check for equally size keys
    if (field_types.size() != key.size()) {
      std::stringstream ss;
      ss << "Flow " << *f << " does not have the same number of keys as"
         << "table: " << *d->name();
      throw Type_error({}, ss.str());    

      return false;  
    }

    // check that each subkey type can be converted
    // to the type specified by the table
    auto table_subtype = field_types.begin();
    auto subkey = key.begin();

    Expr_seq new_key;
    for(;table_subtype != field_types.end() && subkey != key.end(); ++table_subtype, ++subkey) 
    {
      Expr* e = require_converted(elab, *subkey, *table_subtype);
      if (e)
        new_key.push_back(e);
      else {
        std::stringstream ss;
        ss << "Failed type conversion in flow #" << flow_cnt << " subkey: " << **subkey;
        throw Type_error({}, ss.str());
      }
    }

    flow->keys_ = new_key;
  }

  return true;
}


} // namespace


Expr*
Elaborator::elaborate(Add_expr* e)
{
  return check_binary_arithmetic_expr(*this, e);
}


Expr*
Elaborator::elaborate(Sub_expr* e)
{
  return check_binary_arithmetic_expr(*this, e);
}


Expr*
Elaborator::elaborate(Mul_expr* e)
{
  return check_binary_arithmetic_expr(*this, e);
}


Expr*
Elaborator::elaborate(Div_expr* e)
{
  return check_binary_arithmetic_expr(*this, e);
}


Expr*
Elaborator::elaborate(Rem_expr* e)
{
  return check_binary_arithmetic_expr(*this, e);
}


//
Expr*
Elaborator::elaborate(Neg_expr* e)
{
  return check_unary_arithmetic_expr(*this, e);
}


Expr*
Elaborator::elaborate(Pos_expr* e)
{
  return check_unary_arithmetic_expr(*this, e);
}


namespace
{

// The operands of an equality expression are converted
// to rvalues. The operands shall have the same type. The
// result of an equality expression is an rvalue of type
// bool.
//
// TODO: Update equality comparison for new types.
Expr*
check_equality_expr(Elaborator& elab, Binary_expr* e)
{
  // Apply conversions.
  Type const* b = get_boolean_type();
  Expr* e1 = require_value(elab, e->first);
  Expr* e2 = require_value(elab, e->second);

  // Check types.
  if (e1->type() != e2->type())
    throw Type_error({}, "operands have different types");

  e->type_ = b;
  e->first = e1;
  e->second = e2;
  return e;
}

} // naespace


Expr*
Elaborator::elaborate(Eq_expr* e)
{
  return check_equality_expr(*this, e);
}


Expr*
Elaborator::elaborate(Ne_expr* e)
{
  return check_equality_expr(*this, e);
}



namespace
{

// The operands of an ordering expression are converted
// to rvalues. The operands shall have type int. The
// result of an equality expression is an rvalue of type
// bool.
//
// TODO: Update the ordering operands for new types.
Expr*
check_ordering_expr(Elaborator& elab, Binary_expr* e)
{
  // Apply conversions.
  Type const* z = get_integer_type();
  Type const* b = get_boolean_type();
  Expr* c1 = require_converted(elab, e->first, z);
  Expr* c2 = require_converted(elab, e->second, z);
  if (!c1)
    throw Type_error({}, "left operand cannot be converted to 'int'");
  if (!c2)
    throw Type_error({}, "right operand cannot be converted to 'int'");
  
  // Rebuild the expression with the converted
  // operands.
  e->type_ = b;
  e->first = c1;
  e->second = c2;
  return e;
}


} // naespace


Expr*
Elaborator::elaborate(Lt_expr* e)
{
  return check_ordering_expr(*this, e);
}


Expr*
Elaborator::elaborate(Gt_expr* e)
{
  return check_ordering_expr(*this, e);
}


Expr*
Elaborator::elaborate(Le_expr* e)
{
  return check_ordering_expr(*this, e);
}


Expr*
Elaborator::elaborate(Ge_expr* e)
{
  return check_ordering_expr(*this, e);
}


namespace
{


// TODO: Document me!
Expr*
check_binary_logical_expr(Elaborator& elab, Binary_expr* e)
{
  // Apply conversions.
  Type const* b = get_boolean_type();
  Expr* c1 = require_converted(elab, e->first, b);
  Expr* c2 = require_converted(elab, e->second, b);
  if (!c1)
    throw Type_error({}, "left operand cannot be converted to 'bool'");
  if (!c2)
    throw Type_error({}, "right operand cannot be converted to 'bool'");
  
  // Rebuild the expression with the converted
  // operands.
  e->type_ = b;
  e->first = c1;
  e->second = c2;
  return e;
}


// TODO: Document me!
Expr*
check_unary_logical_expr(Elaborator& elab, Unary_expr* e)
{
  Type const* b = get_boolean_type();
  Expr* c = require_converted(elab, e->first, b);
  if (!c)
    throw Type_error({}, "operand cannot be converted to 'bool'");
  
  // Rebuild the expression with the converted
  // operand.
  e->type_ = b;
  e->first = c;
  return e;
}

} // namespace


Expr*
Elaborator::elaborate(And_expr* e)
{
  return check_binary_logical_expr(*this, e);
}


Expr*
Elaborator::elaborate(Or_expr* e)
{
  return check_binary_logical_expr(*this, e);
}


Expr*
Elaborator::elaborate(Not_expr* e)
{
  return check_unary_logical_expr(*this, e);
}


// The target function operand is converted to
// a value and shall have funtion type.
Expr*
Elaborator::elaborate(Call_expr* e)
{
  // Apply lvalue to rvalue conversion and ensure that
  // the target has function type.
  Expr* f = require_value(*this, e->first);

  // If this is just a regular function call.
  // Instead of simply looking for the type, we should do
  // a lookup of the name and check if any functions in scope
  // have that type
  if (Id_expr* id = as<Id_expr>(f)) {
    // maintain list of candidates
    Decl_seq candidates;
    Overload const& ovl = stack.lookup(id->symbol())->second;
    for (auto decl : ovl) {
      if (Function_type const* t = as<Function_type>(decl->type())) {
        // push on as potential candidate
        candidates.push_back(decl);

        // Check for basic function arity.
        Type_seq const& parms = t->parameter_types();
        Expr_seq& args = e->arguments();
        if (args.size() < parms.size())
          continue;
        if (parms.size() < args.size())
          continue;

        // Check that each argument conforms to the the
        // parameter. 

        // FIXME: this is ugly
        bool parms_ok = true;
        for (std::size_t i = 0; i < parms.size(); ++i) {
          Type const* p = parms[i];
          Expr* a = require_converted(*this, args[i], p);
          if (!a)
            parms_ok = false;       
        }
        // reset the loop
        if (!parms_ok) 
          continue;

        // if we get here then this is the correct function
        // The type of the expression is that of the
        // function return type.
        e->type(t->return_type());

        return e;
      }
    }

    // if we get here then no overload resolutions match
    Expr_seq& args = e->arguments();
    std::stringstream ss;
    ss << "No matching function found for call to " << *id->symbol() << "(";
    for (std::size_t i = 0; i < args.size(); ++i) {
      Expr* ai = require_value(*this, args[i]);
      Type const* p = ai->type();

      if (p)
        ss << *p;
      if (i < args.size() - 1)
        ss << ", ";
    }
    ss << ").\n";
    
    // print out all candidates
    ss << "Candidates are: \n";
    for (auto fn : candidates) {
      ss << *fn->name() << *fn->type() << '\n';
    } 
    throw Type_error({}, ss.str());
  }

  // This could potentially be a call whose target is a
  // function object
  Type const* t1 = f->type();
  if (!is<Function_type>(t1))
    throw Type_error({}, "cannot call to non-function");
  Function_type const* t = cast<Function_type>(t1);

  // Check for basic function arity.
  Type_seq const& parms = t->parameter_types();
  Expr_seq& args = e->arguments();
  if (args.size() < parms.size())
    throw Type_error({}, "too few arguments");
  if (parms.size() < args.size())
    throw Type_error({}, "too many arguments");

  // Check that each argument can be converted to each
  // parameter. Rebuild the argument list with the
  // converted arguments.
  //
  // TODO: Note that we actually perform initialization
  // for each argument. How does that interoperate with
  // conversions?
  for (std::size_t i = 0; i < parms.size(); ++i) {
    Type const* p = parms[i];
    Expr* a = require_converted(*this, args[i], p);
    if (!a) {
      std::stringstream ss;
      ss << "type mismatch in argument " << i + 1 << '\n';
      throw Type_error({}, ss.str());
    }
    args[i] = a;
  }

  // The type of the expression is that of the
  // function return type.

  e->type_ = t->return_type();
  e->first = f;
  return e;
}


// TODO: Document the semantics of member access.
//
// TODO: When resolved, this could actually be a
// different kind of expression with a scope and
// a member offset, kind of like an array index.
// We don't need to annotate this expression with
// the position.
Expr*
Elaborator::elaborate(Member_expr* e)
{
  Expr* e1 = elaborate(e->scope());
  if (!is<Reference_type>(e1->type())) {
    std::stringstream ss;
    ss << "cannot access a member of a non-object";
    throw Type_error({}, ss.str());
  }

  // Check the type of the ref

  // Get the non-reference type of the outer
  // object so we can perform lookups.
  Record_type const* t = as<Record_type>(e1->type()->nonref());
  if (!t) {
    std::stringstream ss;
    ss << "object does not have record type";
    throw Type_error({}, ss.str());
  }

  // Re-open the class scope so we can perform lookups
  // in the usual way.
  Record_decl* d = t->declaration();
  Scope_sentinel scope(*this, d);
  for (Decl* d1 : d->fields())
    stack.top().bind(d1->name(), d1);

  // Elaborate the member expression.
  Id_expr* e2 = as<Id_expr>(elaborate(e->member()));
  if (!e2) {
    std::stringstream ss;
    ss << "invalid member reference";
    throw Type_error({}, ss.str());
  }

  // Find the offset in the class of the member.
  // And stash it in the member expression.
  //
  // FIXME: This should be a function.
  for (std::size_t i = 0; i < d->fields().size(); ++i) {
    if (e2->declaration() == d->fields()[i]) {
      e->pos_ = i;
      break;
    }
  }
  assert(e->pos_ != -1);

  // Finally set the type of the expression.
  e->type_ = e2->type();
  e->first = e1;
  e->second = e2;
  return e;
}


// In the expression e1[e2], e1 shall be an object of 
// array type T[N] (for some N) or block type T[]. The
// expression e2 shall be an integer value. The result
// type of the expressions is ref T.
//
// Note that e1 is not converted to a value, and in fact
// *must* be a reference to an object. Converting to a
// value will prevent me from creating element pointers
// in code gen, because we need the base pointer from
// which to compute offsets.
Expr*
Elaborator::elaborate(Index_expr* e)
{
  Expr* e1 = elaborate(e->first);
  if (!is<Reference_type>(e1->type())) {
    std::stringstream ss;
    ss << "cannot index into a value";
    throw Type_error({}, ss.str());
  }

  // Get the non-reference type of the array.
  //
  // FIXME: Does this require a value transformation?
  // We don't (yet?) have array literals, so I generally
  // expect that this *must* be a reference to an array.
  //
  // TODO: Allow block type.
  Array_type const* t = as<Array_type>(e1->type()->nonref());
  if (!t) {
    std::stringstream ss;
    ss << "object does not have array type";
    throw Type_error({}, ss.str());
  }

  // The index shall be an integer value.
  Expr* e2 = require_converted(*this, e->second, get_integer_type());

  // The result type shall be ref T.
  e->type_ = get_reference_type(t->type());
  e->first = e1;
  e->second = e2; 

  return e;
}


// NOTE: Conversions are created after their source
// expressions  have been elaborated. No action is
// required.

Expr*
Elaborator::elaborate(Value_conv* e)
{
  return e;
}


Expr*
Elaborator::elaborate(Block_conv* e)
{
  return e;
}


// TODO: I probably need to elaborate the type.
Expr*
Elaborator::elaborate(Default_init* e)
{
  e->type_ = elaborate(e->type_);
  return e;
}


Expr*
Elaborator::elaborate(Copy_init* e)
{
  // Elaborate the type.
  e->type_ = elaborate(e->type_);

  // Convert the value to the resulting type.
  Expr* c = require_converted(*this, e->first, e->type_);
  if (!c) {
    std::stringstream ss;
    ss << "type mismatch in copy initializer (expected "
       << *e->type() << " but got " << *e->value()->type() << ')';
    throw Type_error({}, ss.str());
  }

  e->first = c;
  return e;
}


// Determine if this is a:
//    - member expr
//    - any future expr that uses a dot
Expr* 
Elaborator::elaborate(Dot_expr* e)
{
  // this is a member expr if the target is a reference type
  Expr* e1 = elaborate(e->target());

  // if its none of the above then we can see if its a member expr
  if (Reference_type const* ref = as<Reference_type>(e1->type())) {
    if (is<Record_type>(ref->nonref())) {
      // construct a member expr
      Expr* mem_expr = new Member_expr(e->target(), e->member());
      // elaborate and return
      return elaborate(mem_expr);
    }
  }

  std::stringstream ss;
  ss << "Unrecognize dot expr: " << *e1;
  throw Type_error({}, ss.str());

  return nullptr;
}



// FIXME: check that the field name
// was extracted if not used in the context
// of an extract decl.
Expr*
Elaborator::elaborate(Field_name_expr* e)
{
  auto binding = stack.lookup(e->name());

  if (!binding) {
    std::stringstream ss;
    ss << *e << " used but not extracted in this decoder.";
    throw Type_error({}, ss.str());
  }

  // maintain every declaration that the field
  // name expr refers to
  Decl_seq decls;
  Expr_seq ids = e->identifiers();

  // confirm that each identifier is a member of the prior element
  // second should always be an element of first, unless first is the
  // last element of the list of identifiers
  if (ids.size() <= 1) {
    throw Type_error({}, "Invalid field name expr with only one valid field.");
  }

  auto first = ids.begin();
  auto second = first + 1;

  Id_expr* id1 = as<Id_expr>(*first);

  // previous
  Layout_decl* prev = nullptr;

  if (id1) {
    Decl* d = stack.lookup(id1->symbol())->second.front();
    if (Layout_decl* lay = as<Layout_decl>(d)) {
      prev = lay;
    }
  }
  else {
    std::stringstream ss;
    ss << "Invalid layout identifier: " << *id1;
    throw Type_error({}, ss.str());
  }

  decls.push_back(prev);

  // the first one is always an identifier to a layout decl
  while(second != ids.end()) {
    if (!prev) {
      std::stringstream ss;
      ss << "Invalid layout identifier: " << *id1;
      throw Type_error({}, ss.str());
    }

    id1 = as<Id_expr>(*first);

    if (!id1) {
      std::stringstream ss;
      ss << "Invalid layout identifier: " << *id1;
      throw Type_error({}, ss.str());
    }

    Id_expr* id2 = as<Id_expr>(*second);

    if (!id2) {
      std::stringstream ss;
      ss << "Invalid layout identifier: " << *id2;
      throw Type_error({}, ss.str());
    }

    // if we can find the field
    // then set prev equal to the new field
    if (Field_decl* f = find_field(prev, id2->symbol())) {
      if (Reference_type const* ref = as<Reference_type>(f->type())) {
        if (Layout_type const* lt = as<Layout_type>(ref->nonref())) {
          prev = lt->declaration();
        }
        else
          prev = nullptr;
      }
      decls.push_back(f);
    }
    else {
      std::stringstream ss;
      ss << "Field " << *id2 << " is not a member of " << *id1;
      throw Type_error({}, ss.str());
    }

    // move the iterators
    ++first;
    ++second;
  }


  e->decls_ = decls;

  // the type is the type of the final declaration
  Type const* t = decls.back()->type();
  if (!t) {
    std::stringstream ss;
    ss << "Field expression" << *e << " of unknown type.";
    throw Type_error({}, ss.str());
  }
  e->type_ = t;

  return e;
}


// -------------------------------------------------------------------------- //
// Elaboration of declarations

// Elaborate a declaration. This returns true if
// elaboration succeeds and false otherwise.
Decl*
Elaborator::elaborate(Decl* d)
{
  struct Fn
  {
    Elaborator& elab;

    Decl* operator()(Variable_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Function_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Parameter_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Record_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Field_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Module_decl* d) const { return elab.elaborate(d); }

    // network declarations
    Decl* operator()(Layout_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Decode_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Table_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Key_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Flow_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Port_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Extracts_decl* d) const { return elab.elaborate(d); }
    Decl* operator()(Rebind_decl* d) const { return elab.elaborate(d); }
  };

  return apply(d, Fn{*this});
}


// The type of the initializer shall match the declared type
// of the variable.
//
// The variable is declared prior to the elaboration of its
// initializer.
Decl*
Elaborator::elaborate(Variable_decl* d)
{
  d->type_ = elaborate(d->type_);

  // Declare the variable.
  // Only declare if its not been forwarded
  // in two pass elaboration.
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);

  // Elaborate the initializer. Note that the initializers
  // type must be the same as that of the declaration.
  d->init_ = elaborate(d->init());

  // Annotate the initializer with the declared
  // object.
  //
  // TODO: This will probably be an expression in
  // the future.
  cast<Init>(d->init())->decl_ = d;

  return d;
}


// The types of return expressions shall match the declared
// return type of the function.
Decl*
Elaborator::elaborate(Function_decl* d)
{
  d->type_ = elaborate(d->type_);

  // Declare the function.
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);

  // Remember if we've seen a function named main().
  //
  // FIXME: This is dumb. We should do this elsewhere
  // in the interpreter.
  if (d->name()->spelling() == "main")
    main = d;

  // Enter the function scope and declare all
  // of the parameters (by way of elaboration).
  //
  // Note that this modifies the origional parameters.
  Scope_sentinel scope(*this, d);
  for (Decl*& p : d->parms_)
    p = elaborate(p);

  // Check the body of the function, if present.
  if (d->body())
    d->body_ = elaborate(d->body());

  // TODO: Are we actually checking returns match
  // the return type?

  // TODO: Build a control flow graph and ensure that
  // every branch returns a value.
  return d;
}


// Elaborate a parameter declaration. This simply declares
// the parameter in the current scope.
Decl*
Elaborator::elaborate(Parameter_decl* d)
{
  d->type_ = elaborate(d->type_);
  stack.declare(d);
  return d;
}


Decl*
Elaborator::elaborate(Record_decl* d)
{
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);

  Scope_sentinel scope(*this, d);
  for (Decl*& f : d->fields_)
    f = elaborate(f);
  return d;
}


Decl*
Elaborator::elaborate(Field_decl* d)
{
  d->type_ = elaborate(d->type_);
  stack.declare(d);
  return d;
}


// Elaborate the module.  Returns true if successful and
// false otherwise.
Decl*
Elaborator::elaborate(Module_decl* m)
{
  Scope_sentinel scope(*this, m);

  // enter a pipeline block
  pipelines.new_pipeline(m);

  // forward declare all module-level declarations
  // so that every name is valid upon seeing it
  forward_declare(m->decls_);

  for (Decl*& d : m->decls_)
    d = elaborate(d);
  return m;
}

// ------------------------------------------------------------ //
//          Network specific declarations


Decl*
Elaborator::elaborate(Layout_decl* d)
{
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);

  Scope_sentinel scope(*this, d);
  for (Decl*& f : d->fields_)
    f = elaborate(f);
  return d;
}


Decl*
Elaborator::elaborate(Decode_decl* d)
{
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);

  pipelines.insert(d);

  if (d->header())
    d->header_ = elaborate(d->header());

  // Enter a scope since a decode body is
  // basically a special function body
  Scope_sentinel scope(*this, d);

  if (d->body())
    d->body_ = elaborate(d->body());

  // TODO: implement me
  return d;
}


Decl*
Elaborator::elaborate(Table_decl* d)
{
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);

  pipelines.insert(d);

  // Tentatively declare that every field
  // needed by the table has been extracted.
  // This is checked later in the pipeline
  // checking phase where all paths leading into
  // this table are analyzed.
  //
  // This allows the elaboration of the key fields
  // to pass without error.
  Scope_sentinel scope(*this, d);

  // maintain the field decl for each field
  // and the type for each field
  Decl_seq field_decls;
  Type_seq types;

  for (auto subkey : d->conditions()) {
    if (Key_decl* field = as<Key_decl>(subkey)) {
      elaborate(field);
      stack.declare(field);

      // construct the table type
      Decl* field_decl = field->declarations().back();

      assert(field_decl);
      assert(field_decl->type());

      // save the field decl
      field_decls.push_back(field_decl);
      // save the type of the field decl
      types.push_back(field_decl->type());
    }
  }

  // elaborate the individual flows
  for (auto flow : d->body()) {
    elaborate(flow);
  }

  Type const* type = get_table_type(field_decls, types);

  d->type_ = type;

  // check initializing flows for type equivalence
  if (!check_table_initializer(*this, d)) {
    std::stringstream ss;
    ss << "Invalid entry in table: " << *d->name();
    throw Type_error({}, ss.str());
  }

  return d;
}


// Elaborating a key decl checks whether
// or not the keys name valid field within
// layouts
Decl*
Elaborator::elaborate(Key_decl* d)
{
  // confirm this occurs within the 
  // context of a table
  if (!is<Table_decl>(stack.context())) {
    std::stringstream ss;
    ss << "Key appearing outside the context of a table declaration: " << *d;
    throw Type_error({}, ss.str());
  }


  // maintain every declaration that the field
  // name expr refers to
  Decl_seq decls;
  Expr_seq ids = d->identifiers();

  // confirm that each identifier is a member of the prior element
  // second should always be an element of first, unless first is the
  // last element of the list of identifiers
  if (ids.size() <= 1) {
    throw Type_error({}, "Invalid key with only one valid field.");
  }

  auto first = ids.begin();
  auto second = first + 1;

  Id_expr* id1 = as<Id_expr>(*first);

  // previous
  Layout_decl* prev = nullptr;

  if (id1) {
    Decl* d = stack.lookup(id1->symbol())->second.front();
    if (Layout_decl* lay = as<Layout_decl>(d)) {
      prev = lay;
    }
  }
  else {
    std::stringstream ss;
    ss << "Invalid layout identifier: " << *id1;
    throw Type_error({}, ss.str());
  }

  decls.push_back(prev);

  // the first one is always an identifier to a layout decl
  while(second != ids.end()) {
    if (!prev) {
      std::stringstream ss;
      ss << "Invalid layout identifier: " << *id1;
      throw Type_error({}, ss.str());
    }

    id1 = as<Id_expr>(*first);

    if (!id1) {
      std::stringstream ss;
      ss << "Invalid layout identifier: " << *id1;
      throw Type_error({}, ss.str());
    }

    Id_expr* id2 = as<Id_expr>(*second);

    if (!id2) {
      std::stringstream ss;
      ss << "Invalid layout identifier: " << *id2;
      throw Type_error({}, ss.str());
    }

    // if we can find the field
    // then set prev equal to the new field
    if (Field_decl* f = find_field(prev, id2->symbol())) {
      // this precludes the very first id from becoming the prev twice
      // since only fields can have reference type and the first is always
      // an identifier to a layout
      //
      // FIXME: there's something not quite correct here
      // What happens when we get a record type which can't belong here?
      if (Reference_type const* ref = as<Reference_type>(f->type())) {
        if (Layout_type const* lt = as<Layout_type>(ref->nonref())) {
          prev = lt->declaration();
        }
        // Does this correctly handle it with the check
        // at the beginning of the loop?
        else
          prev = nullptr; 
      }
      decls.push_back(f);
    }
    else {
      std::stringstream ss;
      ss << "Field " << *id2 << " is not a member of " << *id1;
      throw Type_error({}, ss.str());
    }

    // move the iterators
    ++first;
    ++second;
  }


  d->decls_ = decls;

  // the type is the type of the final declaration
  Type const* t = decls.back()->type();
  if (!t) {
    std::stringstream ss;
    ss << "Field expression" << *d << " of unknown type.";
    throw Type_error({}, ss.str());
  }
  d->type_ = t;

  return d;
}


Decl*
Elaborator::elaborate(Flow_decl* d)
{
  Type_seq types;
  for (auto expr : d->keys()) {
    Expr* key = elaborate(expr);
    types.push_back(key->type());
  }

  d->type_ = get_flow_type(types);

  return d;
}


Decl*
Elaborator::elaborate(Port_decl* d)
{
  if (fwd_set.find(d) == fwd_set.end())
    stack.declare(d);
  // No further elaboration required
  return d;
}


Decl*
Elaborator::elaborate(Extracts_decl* d)
{
  Decode_decl* decoder = as<Decode_decl>(stack.context());
  // guarantee this stmt occurs
  // within the context of a decoder function or a flow function
  if (!decoder) {
    std::stringstream ss;
    ss << "extracts decl " << *d 
       << " found outside of the context of a decoder."
       << "Context is: " << *stack.context()->name();

    throw Type_error({}, ss.str());
  }

  // This should be a guarentee from decoder elaboration
  Layout_type const* header_type = as<Layout_type>(decoder->header());
  assert(header_type);

  Field_name_expr* fld = as<Field_name_expr>(d->field());

  if (!fld) {
    std::stringstream ss;
    ss << "Invalid field name: " << *d->field() << " in extracts decl: " << *d;
    throw Type_error({}, ss.str());
  }

  // declare the extraction with the name
  // of the field to be extracted
  d->name_ = fld->name();
  stack.declare(d);

  Expr* e1 = elaborate(d->field());

  // confirm that the first declaration is
  // the same layout decl that the decoder
  // handles
  if ((fld = as<Field_name_expr>(e1))) {
    if (fld->declarations().front() != header_type->declaration()) {
      std::stringstream ss;
      ss << "Cannot extract from: " << *fld->declarations().front()->name() 
         << " in extracts decl: " << *d
         << ". Decoder " << *decoder->name() << " decodes layout: " << *header_type;
      throw Type_error({}, ss.str());
    }
  }
  else {
    std::stringstream ss;
    ss << "Invalid field name: " << *d->field() << " in extracts decl: " << *d;
    throw Type_error({}, ss.str());
  }

  d->field_ = e1;

  return d;
}


Decl*
Elaborator::elaborate(Rebind_decl* d)
{
  // TODO: implement me
  Expr* e1 = elaborate(d->field1());
  if (!e1) {
    std::stringstream ss;
    ss << "Invalid field name: " << *d->field1() << " in rebind decl: " << *d;
    throw Type_error({}, ss.str());
  }
  d->f1 = e1;

  Expr* e2 = elaborate(d->field2());
  if (!e2) {
    std::stringstream ss;
    ss << "Invalid field name: " << *d->field2() << " in rebind decl: " << *d;
    throw Type_error({}, ss.str());
  }
  d->f2 = e2;

  return d;
}



// -------------------------------------------------------------------------- //
// Elaboration of statements

// Elaborate a statement. This returns true if elaboration
// succeeds and false otherwise.
Stmt*
Elaborator::elaborate(Stmt* s)
{
  struct Fn
  {
    Elaborator& elab;

    Stmt* operator()(Empty_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Block_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Assign_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Return_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(If_then_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(If_else_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Match_stmt* d) const { return elab.elaborate(d); };
    Stmt* operator()(Case_stmt* d) const { return elab.elaborate(d); };
    Stmt* operator()(While_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Break_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Continue_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Expression_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Declaration_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Decode_stmt* d) const { return elab.elaborate(d); }
    Stmt* operator()(Goto_stmt* d) const { return elab.elaborate(d); }
  };

  return apply(s, Fn{*this});
}


Stmt*
Elaborator::elaborate(Empty_stmt* s)
{
  return s;
}


Stmt*
Elaborator::elaborate(Block_stmt* s)
{
  Scope_sentinel scope = *this;
  for (Stmt*& s1 : s->first)
    s1 = elaborate(s1);
  return s;
}


// In an assignment expression, the left operand shall
// refer to a mutable object. The types of the left and
// right operands shall match.
//
// TODO: If we have const types, then we'd have to add this
// checking.
Stmt*
Elaborator::elaborate(Assign_stmt* s)
{
  // FIXME: Write a better predicate?
  Expr* lhs = elaborate(s->object());
  if (!is<Reference_type>(lhs->type()))
    throw Type_error({}, "assignment to rvalue");

  // Apply rvalue conversion to the value and update the
  // expression.
  Expr *rhs = require_value(*this, s->second);

  // The types shall match. Compare t1 using the non-reference
  // type of the object.
  Type const* t1 = lhs->type()->nonref();
  Type const* t2 = rhs->type();
  if (t1 != t2)
    throw Type_error({}, "assignment to an object of a different type");

  s->first = lhs;
  s->second = rhs;
  return s;
}


// The type of the returned expression shall match the declared
// return type of the enclosing function.
//
// TODO: Implement me.
Stmt*
Elaborator::elaborate(Return_stmt* s)
{
  Function_decl* fn = stack.function();
  Type const* t = fn->return_type();

  // Check that the return type matches the returned value.
  Expr* e = elaborate(s->value());
  Expr* c = convert(e, t);
  if (!c) {
    std::stringstream ss;
    ss << "return type mismatch (expected " 
       << *t << " but got " << *s->value()->type() << ")";
    throw std::runtime_error(ss.str());
  }

  s->first = c;
  return s;
}


// The condition must must be a boolean expression.
Stmt*
Elaborator::elaborate(If_then_stmt* s)
{
  Expr* c = require_converted(*this, s->first, get_boolean_type());
  if (!c)
    throw Type_error({}, "if condition does not have type 'bool'");
  Stmt* b = elaborate(s->body());

  s->first = c;
  s->second = b;
  return s;
}


// The condition must must be a boolean expression.
Stmt*
Elaborator::elaborate(If_else_stmt* s)
{
  Expr* c = require_converted(*this, s->first, get_boolean_type());
  if (!c)
    throw Type_error({}, "if condition does not have type 'bool'");
  Stmt* t = elaborate(s->true_branch());
  Stmt* f = elaborate(s->false_branch());

  s->first = c;
  s->second = t;
  s->third = f;
  return s;
}


Stmt* 
Elaborator::elaborate(Match_stmt* s)
{
  Expr* cond = require_converted(*this, s->condition_, get_integer_type());

  if (!cond) {
    std::stringstream ss;
    ss << "Could not convert " << *s->condition_ << " to integer type."; 
    throw Type_error({}, ss.str());
  }

  s->condition_ = cond;

  // TODO: check for same integer type
  // between condition and cases?

  // maintain all values found in every case
  std::unordered_set<Integer_value> vals;

  for (Stmt*& s1 : s->cases_) {
    Stmt* c = elaborate(s1);
    // there cannot be non-case statements inside
    // of a match stmt body
    if (Case_stmt* case_ = as<Case_stmt>(c)) {
      // the case label should be guarenteed to be an integer
      // by elaboration of the case stmt
      if (!vals.insert(case_->label()->value().get_integer()).second) {
        std::stringstream ss;
        ss << "Duplicate label value " << *case_->label() 
           << " found in case statement " << *case_; 
        throw Type_error({}, ss.str());
      }
    }
    else {
      std::stringstream ss;
      ss << "Non-case stmt " << *case_ << " found in match statement."; 
      throw Type_error({}, ss.str());
    }
  }

  return s;
}


// FIXME: make sure case stmts can only appear
// in the context of match
Stmt* 
Elaborator::elaborate(Case_stmt* s)
{
  Expr* label = require_converted(*this, s->label_, get_integer_type());

  if (!is<Literal_expr>(label)) {
    std::stringstream ss;
    ss << "Non-literal value " << *label << " found in case statement."; 
    throw Type_error({}, ss.str());
  }

  if (!is<Integer_type>(label->type())) {
    std::stringstream ss;
    ss << "Non-integer value " << *label << " found in case statement."; 
    throw Type_error({}, ss.str());
  }

  Stmt* stmt = elaborate(s->stmt_);

  s->label_ = label;
  s->stmt_ = stmt;

  return s;
}



Stmt*
Elaborator::elaborate(While_stmt* s)
{
  Expr* c = require_converted(*this, s->first, get_boolean_type());
  if (!c)
    throw Type_error({}, "loop condition does not have type 'bool'");
  Stmt* b = elaborate(s->body());

  s->first = c;
  s->second = b;
  return s;
}


Stmt*
Elaborator::elaborate(Break_stmt* s)
{
  // TODO: Verify that a break occurs within an
  // appropriate context.
  return s;
}


Stmt*
Elaborator::elaborate(Continue_stmt* s)
{
  // TODO: Verify that a continue occurs within an
  // appropriate context.
  return s;
}


Stmt*
Elaborator::elaborate(Expression_stmt* s)
{
  s->first = elaborate(s->expression());
  return s;
}


Stmt*
Elaborator::elaborate(Declaration_stmt* s)
{
  s->first = elaborate(s->declaration());
  return s;
}


Stmt*
Elaborator::elaborate(Decode_stmt* s)
{

  // guarantee this stmt occurs
  // within the context of a decoder function or a flow function
  if (!is<Decode_decl>(stack.context()) && !is<Flow_decl>(stack.context())) {
    std::stringstream ss;
    ss << "decode statement " << *s 
       << " found outside of the context of a decoder or flow.";

    throw Type_error({}, ss.str());
  }

  Expr* target = elaborate(s->decoder_identifier());
  Id_expr* target_id = as<Id_expr>(target);

  if (!target_id) {
    std::stringstream ss;
    ss << "invalid decoder identifier provided in decode statement: " << *s;

    throw Type_error({}, ss.str());
  }

  if (!is<Decode_decl>(target_id->declaration())) {
    std::stringstream ss;
    ss << "invalid decoder " << *target_id
       << " provided in decode statement: " << *s;

    throw Type_error({}, ss.str());
  }

  s->decoder_identifier_ = target_id;
  s->decoder_ = target_id->declaration();

  return s;
}


// TODO: guarantee this stmt occurs
// within the context of a decoder function
Stmt*
Elaborator::elaborate(Goto_stmt* s)
{
  // guarantee this stmt occurs
  // within the context of a decoder function
  if (!is<Decode_decl>(stack.context()) && !is<Flow_decl>(stack.context())) {
    std::stringstream ss;
    ss << "decode statement " << *s 
       << " found outside of the context of a decoder.";

    throw Type_error({}, ss.str());
  }

  Expr* tbl = elaborate(s->table_identifier());

  if (Id_expr* id = as<Id_expr>(tbl)) {
    s->table_identifier_ = tbl;
    s->table_ = id->declaration();
  }
  else {
    std::stringstream ss;
    ss << "invalid table identifier: " << *s->table_identifier();
    throw Type_error({}, ss.str());
  }


  return s;
}


// Copyright (c) 2015 Flowgrammable.org
// All rights reserved

#include "builder.hpp"
#include "evaluator.hpp"
#include "type.hpp"

#include <cmath>
#include <stdexcept>


namespace
{

// -------------------------------------------------------------------------- //
//                              Constant length
//
// Determines if an object type has constant length.

bool has_constant_length(Type const* t);


// A reference type has constant length iff its qualified type
// has constant length.
inline bool
has_constant_length(Reference_type* t)
{
  return has_constant_length(t->type());
}


// A record type has constant length iff all of its members
// have constant length.
inline bool
has_constant_length(Record_type const* t)
{
  // TODO: Use std::all_of.
  for (Decl* d : t->declaration()->fields())
    if (!has_constant_length(d->type()))
      return false;
  return true;
}



// NOTE: The use of the dispatch function keeps the enable_ifs
// out of the signature where they can break the apply() function
// for non-Type hierarchies.
struct Const_fn
{
  template<typename T>
  bool operator()(T const* t) { return dispatch(t); }

  // All scalars have constant length.
  template<typename T>
    typename std::enable_if<is_scalar_type<T>(), bool>::type
  dispatch(T const* t) { return true; }

  // Otherwise, compute recursively.
  template<typename T>
    typename std::enable_if<!is_scalar_type<T>(), bool>::type
  dispatch(T const* t) { return has_constant_length(t); }
};


// Returns true if `t` has constant width.
bool
has_constant_length(Type const* t)
{
  assert(is_object_type(t));
  Const_fn f;
  return apply(t, f);
}


// -------------------------------------------------------------------------- //
//                              Bit precision
//
// Computes the bit precision of a constant-sized type. Behior
// is not defined for types that do not have constant width.

// TODO: There are a lot of computations involving multiples of 8.
// Perhaps we should abstract the notion of bits per byte (CHAR_BIT
// in C/C++) into a separate facility.

int precision(Type const* t);


inline int
precision(Boolean_type const* t)
{
  return 8;
}


// The bit precision of an integer type is explicitly
// given by the type.
inline int
precision(Integer_type const* t)
{
  return t->precision();
}


inline int
precision(Reference_type const* t)
{
  return precision(t->type());
}


// The bit precision of a record is the sum of the
// bit precisions of its member types.
inline int
precision(Record_type const* t)
{
  int n = 0;
  for (Decl* d : t->declaration()->fields())
    n += precision(d->type());
  return n;
}


struct Bits_fn
{
  template<typename T>
  int operator()(T const* t) { return precision(t); }
};


int
precision(Type const* t)
{
  assert(has_constant_length(t));
  Bits_fn f;
  return apply(t, f);
}


// -------------------------------------------------------------------------- //
//                              Byte length
//
// Returns an expression that computes the byte length
// of an object.

Expr* length(Type const*);


// The length of the bolean type is 1.
Expr*
length(Boolean_type const* t)
{
  return one();
}


// Returns an expression that denotes the precison of an
// integer type.
//
// Note the length of integer types within a record is not
// measured by this function. That depends on the bit
// precision of each type.
//
// FIXME: This should actually round to the nearest favorable
// integer alignment for the sake of efficiency.
Expr*
length(Integer_type const* t)
{
  double p = t->precision();
  double w = 8;
  double b = std::ceil(p / w);
  return make_int(b);
}


// The length of a record type is the sum of the precisions
// of its members. Note that we must account for accurate
// bit lengths. If a member has dynamic type, then we need
// to synthesize a call to that member.
Expr*
length(Record_type const* t)
{
  Evaluator eval;

  Expr* e = 0;
  for (Decl* d : t->declaration()->fields()) {
    Type const* t1 = d->type();

    // If member is constant, just add in the constant value
    if (has_constant_length(t1))
      e = add(e, make_int(precision(t1)));

    // Otherwise, we have to form a call to the function
    // that would compute this type.
    else
      // FIXME: Do this right!
      e = add(e, zero());
  }

 
  // Compute ceil(e / 8).
  Expr* b = make_int(8); // bits per byte
  Expr* r = div(sub(add(e, b), one()), b);

  // Try folding the result. If it works, good. If not, 
  // just return the previously computed expression.
  //
  // TODO: Maximally reduce the expression so that we only
  // add the constant bits to the non-constant bits. Since
  // addition is associative and commutative, we can
  // partition the sequence of terms into constants and
  // non-constants, and then sum the constant parts.
  try {
    Value v = eval.eval(r);
    if (v.is_integer())
      return make_int(v.get_integer());
    else
      throw std::runtime_error("failed to synth length");
  }
  catch(...) {
    return r;
  }
}


Expr*
length(Reference_type const* t)
{
  return length(t->type());
}


struct Length_fn
{
  template<typename T>
  Expr* operator()(T const* t) { return dispatch(t); }

  // Fail on non-object types.
  template<typename T>
    static typename std::enable_if<!is_object_type<T>(), Expr*>::type
  dispatch(T const* t)
  {
    throw std::runtime_error("unreachable length");
  }

  template<typename T>
    static typename std::enable_if<is_object_type<T>(), Expr*>::type
  dispatch(T const* t)
  {
    return length(t);
  }
};


Expr*
length(Type const* t)
{
  Length_fn f;
  return apply(t, f);
}


// -------------------------------------------------------------------------- //
//                             Function synthesis

// This module is responsible for the synthesis of a length function
// for a given type. The language requests the lookup and possible
// synthesis of these functions in particular contexts, but that
// lookup is always expressed in terms of type.
//
// The general definition of a length function is:
//
//    def: lengthof(T t) -> uint { ... }
//
// where `T` is the type under consideration.
//
// TODO: Make that a constant reference to T so we don't try
// to copy the record. Naturally, this means we now have to
// support reference types and qualified types. Ugh.



// Build a length function with the following definition.
//
//    def lengthof(t : T) { return <length T>; }
//
// where <length T> is the length of the given type (see
// `length` above).
// struct Synth_fn
// {
//   template<typename T>
//   Decl* operator()(T const* t) { return dispatch(t); }

//   // Fail on non-object types.
//   template<typename T>
//     static typename std::enable_if<!is_object_type<T>(), Decl*>::type
//   dispatch(T const* t)
//   {
//     throw std::runtime_error("unreachable synth");
//   }

//   template<typename T>
//     static typename std::enable_if<is_object_type<T>(), Decl*>::type
//   dispatch(T const* t)
//   {
//     Stmt* r = ret(length(t));
//     Decl* p = make_parm("t", t);
//     return make_fn("lengthof", {p}, get_integer_type(), {r});
//   }
// };


} // namespace


// Synthesize a length function for the given object type.
// Behavior is undefined if `t` is not an object type.
// Decl*
// synthesize_length(Type const* t)
// {
//   assert(is_object_type(t));
//   Synth_fn f;
//   return apply(t, f);
// }


// Returns an expression that computes the length in bytes
// of a packet.
Expr*
get_length(Expr const* e)
{
  return length(e->type());
}

// Returns an expression that computes the length of a
// type in bytes
Expr*
get_length(Type const* t)
{
  return length(t);
}


Expr*
get_length(Layout_decl const* layout)
{
  Evaluator eval;

  Expr* e = 0;
  for (Decl* d : layout->fields()) {
    Type const* t1 = d->type();

    // If member is constant, just add in the constant value
    if (has_constant_length(t1))
      e = add(e, make_int(precision(t1)));

    // Otherwise, we have to form a call to the function
    // that would compute this type.
    else
      // FIXME: Do this right!
      e = add(e, zero());
  }

 
  // Compute ceil(e / 8).
  Expr* b = make_int(8); // bits per byte
  Expr* r = div(sub(add(e, b), one()), b);

  // Try folding the result. If it works, good. If not, 
  // just return the previously computed expression.
  //
  // TODO: Maximally reduce the expression so that we only
  // add the constant bits to the non-constant bits. Since
  // addition is associative and commutative, we can
  // partition the sequence of terms into constants and
  // non-constants, and then sum the constant parts.
  try {
    Value v = eval.eval(r);
    if (v.is_integer())
      return make_int(v.get_integer());
    else
      throw std::runtime_error("failed to synth length");
  }
  catch(...) {
    return r;
  } 
}
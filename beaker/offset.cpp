#include "builder.hpp"
#include "length.hpp"
#include "offset.hpp"
#include "decl.hpp"

namespace steve
{

// Returns an expression which computes the byte offsetof a 
// member within a record.
// 'e' is an offsetof expression.
//
// The byte offset of a member within a record type
// is the sum of the length of all fields preceding
// it within the record.
Expr const* 
get_offset(Decl const* layout, Decl const* mem)
{
  assert(is<Layout_decl>(layout));
  // keep track of all member declarations coming before mem
  Decl_seq pred;

  Expr* offsetof = zero();

  for (auto decl : as<Layout_decl>(layout)->fields()) {
    if (decl == mem) {
      break;
    }

    pred.push_back(decl);
  }

  for (auto decl : pred) {
    offsetof = add(offsetof, get_length(decl->type()));
  }

  return offsetof;
}

} // namespace steve

#ifndef LOWER_HPP
#define LOWER_HPP

#include "builder.hpp"
#include "builtin.hpp"
#include "type.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "stmt.hpp"

namespace steve
{

Stmt_seq lower(Stmt const*);
Expr const* lower(Expr const*);
Decl const* lower(Decl const*);

} // namespace steve

#endif

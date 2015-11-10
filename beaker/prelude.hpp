// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef BEAKER_PRELUDE_HPP
#define BEAKER_PRELUDE_HPP

#include "cast.hpp"
#include "symbol.hpp"

#include "lingo/node.hpp"
#include "lingo/print.hpp"

#include <iosfwd>
#include <vector>
#include <stdexcept>
#include <type_traits>


struct Expr;
struct Literal_expr;
struct Id_expr;
struct Add_expr;
struct Sub_expr;
struct Mul_expr;
struct Div_expr;
struct Rem_expr;
struct Neg_expr;
struct Pos_expr;
struct Eq_expr;
struct Ne_expr;
struct Lt_expr;
struct Gt_expr;
struct Le_expr;
struct Ge_expr;
struct And_expr;
struct Or_expr;
struct Not_expr;
struct Call_expr;
struct Value_conv;
struct Default_init;
struct Copy_init;

struct Type;
struct Id_type;
struct Boolean_type;
struct Integer_type;
struct Function_type;
struct Record_type;
struct Reference_type;
struct Table_type;
struct Flow_type;
struct Port_type;
struct Void_type;
struct Context_type;

struct Decl;
struct Record_decl;
struct Member_decl;
struct Variable_decl;
struct Function_decl;
struct Parameter_decl;
struct Record_decl;
struct Field_decl;
struct Module_decl;
struct Decode_decl;
struct Table_decl;
struct Flow_decl;
struct Extracts_decl;
struct Rebind_decl;
struct Port_decl;

struct Stmt;
struct Empty_stmt;
struct Block_stmt;
struct Assign_stmt;
struct Return_stmt;
struct If_then_stmt;
struct If_else_stmt;
struct Match_stmt;
struct Case_stmt;
struct While_stmt;
struct Break_stmt;
struct Continue_stmt;
struct Expression_stmt;
struct Declaration_stmt;

//      Builtins
struct Bind_offset_expr;
struct Bind_header_expr;
struct Alias_bind_expr;
struct Load_expr;
struct Store_expr;
struct Create_table_expr;
struct Delete_table_expr;
struct Lookup_expr;
struct Advance_expr;
//      Instructions
struct Write_drop_stmt;
struct Write_output_stmt;
struct Goto_stmt;
//      Required Actions
struct Drop_stmt;
struct Output_stmt;
struct Group_expr;


using Expr_seq = std::vector<Expr*>;
using Type_seq = std::vector<Type const*>;
using Decl_seq = std::vector<Decl*>;
using Stmt_seq = std::vector<Stmt*>;


#include "print.hpp"

#endif

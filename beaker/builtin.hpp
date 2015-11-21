#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include "expr.hpp"
#include "stmt.hpp"

// Define a set of global names for each builtin function
constexpr char* __bind_header  = "__bind_header";
constexpr char* __bind_offset  = "__bind_offset";
constexpr char* __double_bind_offset = "__double_bind_offset";
constexpr char* __advance      = "__advance";
constexpr char* __get_context  = "__get_context";
constexpr char* __match        = "__match";
constexpr char* __decode       = "__decode";
constexpr char* __header_cast  = "__header_cast";
constexpr char* __lookup_hdr   = "__lookup_hdr";
constexpr char* __lookup_fld   = "__lookup_fld";
constexpr char* __context_type = "App_cxt";

std::unordered_multimap<String, Function_decl const*> builtin_functions();

Function_decl const* builtin_function(String const);
Record_type const* builtin_type(String const);

Function_decl const* make_match_fn(Type const*);
Function_decl const* get_match_fn(Type const*);

Function_decl const* make_decode_fn(Type const*);
Function_decl const* get_decode_fn(Type const*);

Expr const* make_header_cast(Type const*);

void init_builtins();

// Contains builtin expressions representing the
// flowpath south-bound interface
// i.e. functions which the runtime define and we can link against

// These functions will be linked externally from the C runtime

// Bind the location of an offset
struct Bind_offset_expr : Expr
{};

// Bind the location of a header
struct Bind_header_expr : Expr
{};

// Alias bind of a header
// i.e. extract h1 as h2
struct Alias_bind_expr : Expr
{};

// Loads the value of a field into memory
struct Load_expr : Expr
{};

// Store the value of a field into memory
struct Store_expr : Expr
{};

// Tell the dataplane to create a table
struct Create_table_expr : Expr
{};

// Remove a table
// Why do we need this per se?
struct Delete_table_expr : Expr
{};

// Perform a lookup and execution within a table
struct Lookup_expr : Expr
{};


// Advance the current byte in the table
struct Advance_expr : Expr
{};


// ------------------------------------------------ //
//      Instructions

// Write a drop action
struct Write_drop_stmt : Stmt
{};


// Write an output action
struct Write_output_stmt : Stmt
{};


// Goto the next table
struct Goto_stmt : Stmt
{};


// ------------------------------------------------ //
//      Required Actions


// Drop the packet
struct Drop_stmt : Stmt
{};


// Output the packet
struct Output_stmt : Stmt
{};


// Goto a group table
struct Group_expr : Stmt
{};


#endif

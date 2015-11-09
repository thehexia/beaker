#ifndef FP_SBI
#define FP_SBI

#include "expr.hpp"
#include "stmt.hpp"

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
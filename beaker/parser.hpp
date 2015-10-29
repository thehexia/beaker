// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#ifndef BEAKER_PARSER_HPP
#define BEAKER_PARSER_HPP

#include "prelude.hpp"
#include "string.hpp"
#include "token.hpp"


class Input_buffer;


// The parser performs syntactic analysis, transforming
// a token stream into an AST.
class Parser
{
public:
  Parser(Symbol_table&, Token_stream&);
  Parser(Symbol_table&, Token_stream&, Location_map&);

  // Expression parsers
  Expr* primary_expr();
  Expr* call_expr();
  Expr* postfix_expr();
  Expr* unary_expr();
  Expr* multiplicative_expr();
  Expr* additive_expr();
  Expr* ordering_expr();
  Expr* equality_expr();
  Expr* logical_and_expr();
  Expr* logical_or_expr();
  Expr* expr();

  // Type parsers
  Type const* type();

  // Declaration parsers
  Decl* decl();
  Decl* variable_decl();
  Decl* parameter_decl();
  Decl* function_decl();

  // Statement parsers
  Stmt* stmt();
  Stmt* empty_stmt();
  Stmt* block_stmt();
  Stmt* return_stmt();
  Stmt* if_stmt();
  Stmt* for_stmt();
  Stmt* declaration_stmt();
  Stmt* expression_stmt();

  // Top-level.
  Decl* module();

  // Parse state
  bool ok() const { return errs_ == 0; }

  explicit operator bool() const { return ok(); }

private:
  // Actions
  Expr* on_id(Token);
  Expr* on_bool(Token);
  Expr* on_int(Token);
  Expr* on_add(Expr*, Expr*);
  Expr* on_sub(Expr*, Expr*);
  Expr* on_mul(Expr*, Expr*);
  Expr* on_div(Expr*, Expr*);
  Expr* on_rem(Expr*, Expr*);
  Expr* on_neg(Expr*);
  Expr* on_pos(Expr*);
  Expr* on_eq(Expr*, Expr*);
  Expr* on_ne(Expr*, Expr*);
  Expr* on_lt(Expr*, Expr*);
  Expr* on_gt(Expr*, Expr*);
  Expr* on_le(Expr*, Expr*);
  Expr* on_ge(Expr*, Expr*);
  Expr* on_and(Expr*, Expr*);
  Expr* on_or(Expr*, Expr*);
  Expr* on_not(Expr*);
  Expr* on_call(Expr*, Expr_seq const&);

  Decl* on_variable_decl(Token, Type const*, Expr*);
  Decl* on_parameter_decl(Token, Type const*);
  Decl* on_function_decl(Token, Decl_seq const&, Type const*, Stmt*);
  Decl* on_module_decl(Decl_seq const&);

  // FIXME: Remove _stmt from handlers.
  Stmt* on_empty();
  Stmt* on_block(Stmt_seq const&);
  Stmt* on_assign(Expr*, Expr*);
  Stmt* on_return(Expr*);
  Stmt* on_if_then(Expr*, Stmt*);
  Stmt* on_if_else(Expr*, Stmt*, Stmt*);
  Stmt* on_for(Decl*, Expr*, Expr*);
  Stmt* on_expression(Expr*);
  Stmt* on_declaration(Decl*);

  // Parsing support
  Token_kind lookahead() const;
  Token      match(Token_kind);
  Token      match_if(Token_kind);
  Token      require(Token_kind);
  Token      accept();
  void       consume_until(Token_kind);
  void       consume_thru(Token_kind);

  // Error handling. These functions throw
  // exceptions. They never return.
  [[noreturn]] void error(char const*);
  [[noreturn]] void error(String const&);

  template<typename T, typename... Args>
  T* init(Location, Args&&...);

private:
  Symbol_table& syms_;
  Token_stream& ts_;
  Location_map* locs_;

  int errs_;        // Error count

  // Parse flags
  Token_kind term_; // Statement termination token.
};


inline
Parser::Parser(Symbol_table& s, Token_stream& t)
  : syms_(s), ts_(t), locs_(nullptr), errs_(0), term_()
{ }


inline
Parser::Parser(Symbol_table& s, Token_stream& t, Location_map& l)
  : syms_(s), ts_(t), locs_(&l), errs_(0), term_()
{ }


// Returns the first token of lookahead.
inline Token_kind
Parser::lookahead() const
{
  return Token_kind(ts_.peek().kind());
}


// A helper function to create nodes and record their
// source location.
//
// TODO: Put this in the .cpp file? It is private.
template<typename T, typename... Args>
inline T* 
Parser::init(Location loc, Args&&... args)
{
  T* t = new T(std::forward<Args>(args)...);
  locs_->emplace(t, loc);
  return t;
}


#endif
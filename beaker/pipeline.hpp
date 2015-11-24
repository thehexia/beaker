
#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "prelude.hpp"
#include "elaborator.hpp"

#include <unordered_set>
#include <unordered_map>
#include <set>

// This module is used to connect the decode decls
// togethers. This allows us to formt he graph to
// type check the flow tables

struct Elaborator;
struct Stage;

using Decl_set = std::unordered_set<Decl const*>;
using Sym_set = std::unordered_set<Symbol const*>;
using Stage_set = std::unordered_set<Stage const*>;


struct Field_env : Environment<Symbol const*, Extracts_decl const*>
{
  Field_env()
    : decl_(nullptr)
  { }

  Field_env(Extracts_decl* d)
    : decl_(d)
  { }

  Extracts_decl* decl_;
};


// Map headers to integers
struct Header_map : std::unordered_map<Layout_decl const*, int>
{
  Header_map()
    : count(0)
  { }

  void insert(Layout_decl const* l);

  int count;
};


// Map fields to integers
struct Field_map : std::unordered_map<Symbol const*, int>
{
  Field_map()
    : count(0)
  { }

  void insert(Extracts_decl const* e);

  int count;
};


struct Stage_stack : Stack<Field_env>
{
  using Stack<Field_env>::Stack;

  void produce(Field_env const& f) { push(f); }
};


// Represents a stage in the pipeline
// This can be either a decode decl or a table decl
struct Stage
{
  Stage(Decl const*, Stage_set const&, Field_env const&, Sym_set const&);

  Decl const* decl() const { return stage_; }
  Stage_set const& branches() const { return branches_; }
  Field_env const& productions() const { return products_; }
  Sym_set const& requirements() const { return reqs_; }

  Decl const* stage_;
  Stage_set branches_;
  Field_env products_;
  Sym_set reqs_;

  // for dfs
  bool visited;
};


struct Stage_less
{
  bool operator()(Stage const& a, Stage const& b) const
  {
    return a.decl() < b.decl();
  }
};


// A list of pipeline stages
struct Pipeline : std::vector<Stage*>
{
  Stage* find(Decl const*) const;
};



struct Pipeline_checker
{
  struct Stage_sentinel;

  Pipeline_checker(Elaborator& e)
    : elab(e),  entry(nullptr), is_error_state(false)
  { }

  bool check_pipeline();
  void extract(Extracts_decl*);
  void header(Layout_decl*);

  // Uncover all branches within a stage
  // Branches can occur only within stmts.
  // Branches can not happen as a result of a function call.
  void discover_branches();
  Stage_set find_branches(Decode_decl const*);
  Stage_set find_branches(Table_decl const*);
  Stage_set find_branches(Flow_decl const*);

  // Register a decoding stage
  void register_stage(Decode_decl const*);
  void register_stage(Table_decl const*);
  void register_stage(Flow_decl const*);

  // Discover all productions
  Field_env get_productions(Decode_decl const*);
  Field_env get_productions(Table_decl const*);
  Field_env get_productions(Flow_decl const*);

  // Discover all requirements
  Sym_set get_requirements(Decode_decl const*);
  Sym_set get_requirements(Table_decl const*);
  Sym_set get_requirements(Flow_decl const*);

  void print_header_mappings();
  void print_field_mappings();
  void print_stages();
  void print_stage(Stage const*);

  int get_header_mapping(Layout_decl const*);
  int get_field_mapping(Symbol const*);

private:
  void check_stage(Decl const*, Sym_set const&);
  void dfs(Stage*);

  // Map headers to integers
  Header_map hdr_map;

  // Map fields to integers
  Field_map fld_map;

  // maintain a stack of fields
  // extracted
  Stage_stack stack;

  // Maintain a pipeline
  Pipeline pipeline;

  // maintain the original elaborator
  Elaborator& elab;

  // Maintain the starting stage
  Stage* entry;

  // Maintain if this is in error state
  bool is_error_state;

  // Maintain the current path for debugging purposes
  std::vector<Stage*> path;
};


struct Pipeline_checker::Stage_sentinel
{
  Stage_sentinel(Pipeline_checker& p, Extracts_decl* d = nullptr)
    : checker(p)
  {
    checker.stack.push(d);
  }

  ~Stage_sentinel()
  {
    checker.stack.pop();
  }

  Pipeline_checker& checker;
};


// struct Extracted : std::vector<Decl const*>
// {
//   Extracted(int c, Decl const* d, String const* n)
//     : std::vector<Decl const*>::vector { d }, count(c), name_(n)
//   { }

//   bool is_singleton() const { return size() == 1; }
//   String const* name() const { return name_; }
//   Decl const* latest() const { return back(); }

//   void pop() { pop_back(); }
//   void push(Decl const* d) { push_back(d); }

//   int count;
//   String const* name_;
// };


// // Field environment
// struct Pipeline_environment : std::unordered_map<String const*, Extracted*>
// {
//   void       push(String const*, Decl const*);
//   Extracted* pop(String const*);

//   Extracted* lookup(String const*);
// };


// // Used to keep track of valid field/header identifiers
// // when checking the pipeline. We don't really care about
// // which declaration they refer to, only that the identifier is
// // a valid name
// struct Context_bindings : std::unordered_set<String const*>
// {
//   using std::unordered_set<String const*>::unordered_set;
// };


// // Keeps track of all possible fields extracted
// // and all headers extracted in the program
// struct Context_environment
// {
//   Pipeline_environment& headers() { return first; }
//   Pipeline_environment& fields() { return second; }

//   // header environment
//   Pipeline_environment first;
//   Pipeline_environment second;
// };

// void register_stage(Decode_decl const*);
// void register_stage(Table_decl const*);
// bool check_pipeline();

// Value lookup_field_binding(String const*);
// Value lookup_header_binding(String const*);

// Pipeline const& get_pipeline();

// int get_num_headers();
// int get_num_fields();

// // get the integer representation for a field
// Value_expr* get_header_binding(String const*);
// Value_expr* get_field_binding(String const*);

// Decl const* pipeline_get_start();

#endif

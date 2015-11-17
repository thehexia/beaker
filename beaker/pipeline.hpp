
#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "prelude.hpp"
#include "elaborator.hpp"

#include <unordered_set>

// This module is used to connect the decode decls
// togethers. This allows us to formt he graph to
// type check the flow tables

struct Elaborator;


// Represents a stage in the pipeline
// This can be either a decode decl or a table decl
struct Stage
{  
  using Sym_set = std::unordered_set<Symbol const*>;
  using Stage_set = std::unordered_set<Stage const*>;

  Stage(Decl const*, Stage_set const&, Sym_set const&);

  Decl const* decl() const { return stage_; }
  Sym_set const& requirements() const { return reqs_; }
  Stage_set const& branches() const { return branches_; }
  Sym_set const& productions() const { return products_; }

  Decl const* stage_;
  Sym_set reqs_;
  Stage_set branches_;
  Sym_set products_;

  // for dfs
  bool visited;
};


// A list of pipeline stages
struct Pipeline : std::vector<Stage*>
{
  Stage* find(Decl const*) const;
};



struct Field_env : Environment<Symbol const*, Extracts_decl*>
{
  Field_env() 
    : decl_(nullptr)
  { }

  Field_env(Extracts_decl* d) 
    : decl_(d)
  { }

  Extracts_decl* decl_;
};


struct Header_env : Environment<Symbol const*, Layout_decl*>
{
  using Environment<Symbol const*, Layout_decl*>::Environment;
};


struct Stage_stack : Stack<Field_env>
{
  using Stack<Field_env>::Stack;
};


struct Pipeline_checker
{
  using Sym_set = std::unordered_set<Symbol const*>;
  using Stage_set = std::unordered_set<Stage const*>;

  struct Stage_sentinel;

  Pipeline_checker(Elaborator& e)
    : elab(e)
  { }

  bool check_pipeline();
  void extract(Extracts_decl*);
  void header(Layout_decl*);

  // Uncover all branches within a stage
  // Branches can occur only within stmts.
  // Branches can not happen as a result of a function call.
  Stage_set find_branches(Decode_decl*);
  Stage_set find_branches(Table_decl*);

  // Register a decoding stage
  void register_stage(Decode_decl*);
  void register_stage(Table_decl*);

  // Discover all productions
  Sym_set get_productions(Decode_decl*);
  Sym_set get_productions(Table_decl*);

  // Discover all requirements
  Sym_set get_requirements(Decode_decl*);
  Sym_set get_requirements(Table_decl*);

private:
  // record field and header
  // extractions
  Field_env fld_env;
  Header_env hdr_env;

  // maintain a stack of fields
  // extracted
  Stage_stack stack;

  // Maintain a pipeline
  Pipeline p;

  // maintain the original elaborator
  Elaborator& elab;
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


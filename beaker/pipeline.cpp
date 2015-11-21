#include "type.hpp"
#include "expr.hpp"
#include "decl.hpp"
#include "stmt.hpp"
#include "pipeline.hpp"
#include "error.hpp"

#include <iostream>


void
Pipeline_checker::check_stage(Decl const* d, Sym_set const& reqs)
{
  for (auto field : reqs) {
    auto search = stack.lookup(field);

    if (!search) {
      std::stringstream ss;
      ss << "Invalid field requirement. Field " << *field
         << "required but not decoded.\n";

      ss << "Broken path: ";
      for (auto stage : path)
        ss << stage->decl()->name();

      throw Lookup_error({}, ss.str());
    }
  }
}

// Depth first search
// specialized to visit ALL PATHS within a
// graph instead of visiting all nodes within
// a graph.
void
Pipeline_checker::dfs(Stage* s)
{
  s->visited = true;

  // push all of its productions onto the bindings stack
  stack.produce(s->productions());

  // FIXME: Do we even care what headers are valid at any
  // given stage? It seems to me we only ever
  // care about the fields
  // push the header name onto the bindings stack
  // cxt_bindings.insert(s->decl()->name());

  // push stage onto path for debugging purposes
  path.push_back(s);

  // check this stage
  check_stage(s->decl(), s->requirements());

  for (auto br : s->branches()) {
    if (br->decl() != s->decl()) {
      std::cout << "Decl1: " << *br->decl()->name() << " " << br->decl();
      std::cout << "Decl2: " << *s->decl()->name() << " " << s->decl();

      Stage* stage = pipeline.find(br->decl());
      // if (stage)
      //   if (!stage->visited)
      //     dfs(stage);
    }
  }

  // cleanup
  // pop off debugging stack
  path.pop_back();

  // pop all of the bindings off
  stack.pop();

  // pop the header name off bindings stack
  // cxt_bindings.erase(s->decl()->name());

  // unset the visited as you come back from recursion
  // so that we can explore all possible paths instead of
  // just one path
  s->visited = false;
}


void
Pipeline_checker::print_header_mappings()
{
  std::cout << "======== Header-mapping =========\n";
  for (auto mapping : hdr_map) {
    std::cout << *mapping.first->name() << " : " << mapping.second << '\n';
  }
}


void
Pipeline_checker::print_field_mappings()
{
  std::cout << "======== Field-mapping =========\n";
  for (auto mapping : fld_map) {
    std::cout << *mapping.first << " : " << mapping.second << '\n';
  }
}



void
Pipeline_checker::print_stages()
{
  std::cout << "======== Stages =========\n";
  for (auto stage : pipeline) {
    print_stage(stage);
  }
  std::cout << "=================\n";
}


void
Pipeline_checker::print_stage(Stage const* s)
{
  std::cout << "\nStage: \n";
  std::cout << *s->decl()->name() << '\n';
  std::cout << "productions: \n";
  for (auto sym : s->productions()) {
    std::cout << *sym.first << " ";
  }
  std::cout << '\n';

  std::cout << "branches: \n";

  for (auto b : s->branches()) {
    std::cout << *b->decl()->name() << " ";
  }
  std::cout << '\n';

  std::cout << "reqs: \n";

  for (auto r : s->requirements()) {
    std::cout << *r << " ";
  }
  std::cout << '\n';
}


// We construct the stage by determining all
// requirements needed before moving into that stage
Stage::Stage(Decl const* d, Stage_set const& b, Field_env const& p, Sym_set const& r)
  : stage_(d), branches_(b), products_(p), reqs_(r), visited(false)
{ }


void
Field_map::insert(Extracts_decl const* e)
{
  assert(e);

  auto ins = this->emplace(e, count);
  // check for insert already
  if (ins.second)
    ++count;
}


void
Header_map::insert(Layout_decl const* l)
{
  assert(l);

  auto ins = this->emplace(l, count);
  // check for insert already
  if (ins.second)
    ++count;
}


// FIXME: this is a linear search
// might be able to transform this into a map for fastter
// search time.
//
// returns pointer to stage if found or
// nullptr otherwise
Stage*
Pipeline::find(Decl const* d) const
{
  for (auto stage : *this) {
    if (stage->decl() == d)
      return stage;
  }
  return nullptr;
}


// Push an extracted field onto stack
void
Pipeline_checker::extract(Extracts_decl* d)
{
  // check if this has already been extracted
  // no reason to register an extraction
  // more than once
  if (!stack.top().lookup(d->name()))
    stack.top().bind(d->name(), d);

  // every time we register an extraction
  // we also map the field to an integer
  // if it has not been already
  fld_map.insert(d);
}


// Register a decode decl
void
Pipeline_checker::register_stage(Decode_decl const* d)
{
  Block_stmt const* body = as<Block_stmt>(d->body());

  if (!body) {
    throw Type_error({}, "Invalid decoder body.");
  }

  // bind the header decl into header map
  Layout_type const* layout_type = as<Layout_type>(d->header());
  hdr_map.insert(layout_type->declaration());

  // get productions
  Field_env product = get_productions(d);

  // get requirements
  // Right now, decoders do not have explicit requirements
  // as they do no care about what happened in the previous
  // decoder. Only tables have requirements.
  Sym_set requirements = get_requirements(d);

  Stage* stage = new Stage(d, Stage_set(), product, requirements);

  assert(d->name());

  if (d->is_start()) {
    if (!entry)
      entry = stage;
    else {
      std::cerr << "Multiple entry points found in pipeline.\n";
      std::cerr << "   First start: " << *entry->decl()->name() << '\n';
      std::cerr << "   Second start: " << *d->name() << '\n';
      is_error_state = true;
    }
  }

  pipeline.push_back(stage);
}


// Register a table decl
void
Pipeline_checker::register_stage(Table_decl const* d)
{
  // Keep track of what fields each stage produces
  // TODO: For now, tables do not produce,
  // but potentially pushing and popping headers
  // causes a production. Though its somewhat questionable
  // if we can guarantee this at compile time.
  Field_env product = get_productions(d);

  // Keep track of requirements
  // Requirements are specified by the declared
  // keys in the table.
  Sym_set requirements = get_requirements(d);

  // check that the table had a default initialization
  if (d->body().size() > 0) {
    // find branches inside flow decl
    for (auto f : d->body()) {
      // check for flow decl
      Flow_decl const* flow = as<Flow_decl>(f);
      // find_branch(flow->instructions());
    }
  }

  Stage* stage = new Stage(d, Stage_set(), product, requirements);

  if (d->is_start()) {
    if (!entry)
      entry = stage;
    else {
      is_error_state = true;
      std::cerr << "This table is a multiple entry points found in pipeline.\n";
    }
  }

  pipeline.push_back(stage);
}


// Get productions of a decode declaration
//
// These correspond to extracted fields.
// We only record the names which get extracted
// and save the declaration elsewhere.
Field_env
Pipeline_checker::get_productions(Decode_decl const* d)
{
  Block_stmt const* body = as<Block_stmt>(d->body());

  assert(body);

  Field_env products;

  struct Find_products
  {
    Field_env& prod;
    Field_map& fld_map;

    void operator()(Empty_stmt const* s) { }
    void operator()(Block_stmt const* s) { }
    void operator()(Assign_stmt const* s) { }
    void operator()(Break_stmt const* s) { }
    void operator()(Continue_stmt const* s) { }
    void operator()(Expression_stmt const* s) { }
    void operator()(Return_stmt const* s) { throw Type_error({}, "return found in decoder body"); }
    void operator()(If_then_stmt const* s) { }
    void operator()(If_else_stmt const* s) { }
    void operator()(Match_stmt const* s) { }
    void operator()(Case_stmt const* s) { }
    void operator()(While_stmt const* s) { }
    void operator()(Decode_stmt const* s) { }
    void operator()(Goto_stmt const* s) { }

    // the only productions (for now) come out of decl statements
    // and only if it is an extracts decl or rebind decl
    void operator()(Declaration_stmt const* s)
    {
      if (Extracts_decl const* ext = as<Extracts_decl>(s->declaration())) {
        prod.emplace(ext->name(), ext);
        fld_map.insert(ext);
      }
      else if (Rebind_decl const* reb = as<Rebind_decl>(s->declaration())) {
        // bind both the original name
        // and the aliased name
      }
    }
  };

  // scan through the decode decl body
  //    extract decls will add fields to the context
  //    header type will be added to the context
  for (auto stmt : body->statements()) {
    apply(stmt, Find_products{products, fld_map});
  }

  return products;
}


// Not sure we can discover if tables produce anything
Field_env
Pipeline_checker::get_productions(Table_decl const* d)
{
  return Field_env();
}


Sym_set
Pipeline_checker::get_requirements(Table_decl const* d)
{
  // Keep track of requirements
  // Requirements are specified by the declared
  // keys in the table.
  Sym_set requirements;
  for (auto subkey : d->conditions()) {
    requirements.insert(subkey->name());
  }

  return requirements;
}


// Decoders don't explicitly require anything
Sym_set
Pipeline_checker::get_requirements(Decode_decl const* d)
{
  Sym_set requirements;

  return requirements;
}


// Branches can only occur within the context
// of flows
Stage_set
Pipeline_checker::find_branches(Table_decl const* d)
{
  Stage_set branches;

  struct Find_branches
  {
    Stage_set& br;
    Pipeline& p;

    // these cannot appear in flow bodies
    void operator()(Empty_stmt const* s) { }
    void operator()(Assign_stmt const* s) { }
    void operator()(Break_stmt const* s) { }
    void operator()(Continue_stmt const* s) { }
    void operator()(Expression_stmt const* s) { }
    void operator()(Declaration_stmt const* s) { }
    void operator()(Return_stmt const* s) { throw Type_error({}, "return found in flow body"); }
    void operator()(Block_stmt const* s) { }
    void operator()(If_then_stmt const* s) { }
    void operator()(If_else_stmt const* s) { }
    void operator()(Match_stmt const* s) { }
    void operator()(Case_stmt const* s) { }
    void operator()(While_stmt const* s) { }

    // these can cause branches
    void operator()(Decode_stmt const* s)
    {
      Stage* b = p.find(s->decoder());
      assert(b);
      br.insert(b);
    }

    void operator()(Goto_stmt const* s)
    {
      Stage* b = p.find(s->table());
      assert(b);
      br.insert(b);
    }
  };

  for (auto f : d->body()) {
    // Flow_decl
    Flow_decl* flow = as<Flow_decl>(f);

    // either its a block stmt
    Block_stmt const* body = as<Block_stmt>(flow->instructions());
    if (body) {
      for (auto stmt : body->statements()) {
        apply(stmt, Find_branches{branches, pipeline});
      }
    }

    // or an identifier to a special action set
    // TODO: implement
  }

  return branches;
}


// Branches can occur within a number of places within a decoder
// decl
Stage_set
Pipeline_checker::find_branches(Decode_decl const* d)
{
  Stage_set branches;

  struct Find_branches
  {
    Stage_set& br;
    Pipeline& p;

    // these do not cause branches
    void operator()(Empty_stmt const* s) {  }
    void operator()(Assign_stmt const* s) {  }
    void operator()(Break_stmt const* s) {  }
    void operator()(Continue_stmt const* s) {  }
    void operator()(Expression_stmt const* s) {  }
    void operator()(Declaration_stmt const* s) {  }
    void operator()(Return_stmt const* s)
    {
      throw Type_error({}, "return found in decoder body");
    }

    // these can cause branches
    void operator()(Block_stmt const* s)
    {
      for (auto stmt : s->statements()) {
        apply(stmt, *this);
      }
    }

    void operator()(If_then_stmt const* s)
    {
      apply(s->body(), *this);
    }

    void operator()(If_else_stmt const* s)
    {
      apply(s->true_branch(), *this);
    }

    void operator()(Match_stmt const* s)
    {
      for (auto c : s->cases()) {
        apply(c, *this);
      }
    }

    void operator()(Case_stmt const* s)
    {
      apply(s->stmt(), *this);
    }

    void operator()(While_stmt const* s)
    {
      apply(s->body(), *this);
    }

    void operator()(Decode_stmt const* s)
    {
      Stage* b = p.find(s->decoder());
      assert(b);
      br.insert(b);
    }

    void operator()(Goto_stmt const* s)
    {
      Stage* b = p.find(s->table());
      assert(b);
      br.insert(b);
    }
  };

  apply(d->body(), Find_branches{branches, pipeline});

  return branches;
}


// Go through all stages in the pipeline
// and discover the branches which it goes to.
void
Pipeline_checker::discover_branches()
{
  for (auto stage : pipeline) {
    if (Decode_decl const* decoder = as<Decode_decl>(stage->decl()))
      stage->branches_ = find_branches(decoder);
    else if (Table_decl const* table = as<Table_decl>(stage->decl()))
      stage->branches_ = find_branches(table);
    else
      throw std::runtime_error("Invalid decl found in pipeline.");
  }
}


bool
Pipeline_checker::check_pipeline()
{
  // first get the pipelines from the elaborator
  // FIXME: for now we only handle one pipeline
  Pipeline_decls pipeline_decls = elab.pipelines.front();

  for (Decl const* d : pipeline_decls) {

    if (Table_decl const* table = as<Table_decl>(d)) {
      register_stage(table);
    }
    else if (Decode_decl const* decode = as<Decode_decl>(d)) {
      register_stage(decode);
    }
    else
      throw std::runtime_error("Unknown pipeline stage.");
  }


  // discover all branches
  discover_branches();

  // check PATHS
  // dfs(entry);

  return false;
}



// #include <iostream>

// namespace steve
// {

// Extracted*
// Pipeline_environment::lookup(String const* n)
// {
//   auto iter = find(n);
//   if (iter != end())
//     return iter->second;
//   else
//     return nullptr;
// }


// Stage*
// Pipeline::find(Decl const* d) const
// {
//   for (Stage* s : *this) {
//     if (s->decl() == d)
//       return s;
//   }
//   return nullptr;
// }


// // TODO: delete me when done testing
// void
// print(Stage* s)
// {
//   print("Stage: ");
//   print(s->decl()->name());
//   print("Branches: ");
//   for (auto b : s->branches()) {
//     print(b->name());
//   }

//   print("\n\n");
// }


// // TODO: delete me when done testing
// void
// print(Extracted* e) {
//   print("Extracted: {}. {} size: {} straddr: ", e->count, e->name(), e->size());
//   std::cout << e->name();
//   print("\n");
// }


// namespace
// {

// // Important elements to the pipeline
// Context_bindings cxt_bindings;
// Context_environment cxt_env;
// Pipeline pipeline(cxt_bindings, cxt_env);
// // maintain the first stage in the pipeline
// Stage* entry = nullptr;
// // maintain if any component in the pipeline is in error
// bool is_error_state = false;


// void
// print_header_env()
// {
//   print("==========Headers=========");
//   for (auto s : pipeline.env().headers()) {
//     print(s.second);
//   }
// }


// void
// print_field_env()
// {
//   print("==========Fields=========");
//   for (auto s : pipeline.env().fields()) {
//     print(s.second);
//   }
// }


// struct Stage_less
// {
//   bool operator()(Stage const& a, Stage const& b) const
//   {
//     return less(a.decl(), b.decl());
//   }
// };


// struct Extracted_less
// {
//   bool operator()(Extracted const& a, Extracted const& b) const
//   {
//     return a.name() < b.name();
//   }
// };


// Unique_factory<Stage, Stage_less> stages_;
// Unique_factory<Extracted, Extracted_less> extracted_;

// // Let's us print the stack leading up to errors
// // in requirement detection
// std::vector<Stage*> stack_;


// } // namespace

// //---------------------------------------------------------------//
// //                 Scoping functions


// void
// Pipeline_environment::push(String const* n, Decl const* d)
// {
//   auto search = this->find(n);

//   if (search != this->end()) {
//     (*search).second->push(d);
//   }
//   else {
//     Extracted* e = extracted_.make(this->size(), d, n);
//     this->insert(std::make_pair(n, e));
//   }
// }


// Extracted*
// Pipeline_environment::pop(String const* n)
// {
//   auto search = this->find(n);

//   if (search != this->end()) {
//     auto ret = (*search).second;
//     this->erase(search);
//     return ret;
//   }

//   return nullptr;
// }


// namespace
// {


// // Table requirements are determined by the match fields
// // which they require to be decoded
// void
// table_requirements(Table_decl const* d, Expr_seq& req)
// {
//   for (auto r : d->conditions()) {
//     if(!is<Field_expr>(r))
//       error(r->location(), "Invalid field requirement '{}'.", r);

//     req.push_back(r);
//   }
// }


// // TODO: Determine if decoders even have explicit requirements
// void
// decode_requirements(Decode_decl const* d, Expr_seq& req)
// {

// }


// // handle extract decls
// void
// register_extract(Extracts_decl const* d, Expr_seq& product)
// {
//   // push the value onto the productions of the stage
//   product.push_back(d->field());

//   pipeline.env().fields().push(as<Field_expr>(d->field())->name(), d);
// }


// // A rebind declaration registers the extracted field
// // into the header environment using the aliased name and the original
// // i.e. extract vlan.type as eth.type
// // creates an entry with the name eth.type and
// // creates an entry with the name vlan.type
// void
// register_rebind(Rebind_decl const* d, Expr_seq& product)
// {
//   // push the value onto the productions of the stage
//   // push both onto the possible products because it may
//   // be refered to as either later
//   product.push_back(d->field1());
//   product.push_back(d->field2());

//   pipeline.env().fields().push(as<Field_expr>(d->field1())->name(), d);
//   pipeline.env().fields().push(as<Field_expr>(d->field2())->name(), d);
// }


// // Only decode decls will have products
// // the only decls that produce are extracts and rebind decls
// void
// find_productions(Stmt const* s, Expr_seq& product)
// {
//   if (is<Decl_stmt>(s)) {
//     Decl_stmt const* ds = as<Decl_stmt>(s);

//     if (is<Extracts_decl>(ds->decl()))
//       register_extract(as<Extracts_decl>(ds->decl()), product);
//     if (is<Rebind_decl>(ds->decl()))
//       register_rebind(as<Rebind_decl>(ds->decl()), product);
//   }
// }


// // ------------------------------------------------------------- //
// //          Check branches within a decoder

// void find_branch(Block_stmt const*, Decl_set&);
// void find_branch(Expr_stmt const*, Decl_set&);
// void find_branch(Match_stmt const*, Decl_set&);
// void find_branch(Case_stmt const*, Decl_set&);
// void find_branch(Stmt const*, Decl_set&);

// void find_branch(Flow_decl const*, Decl_set&);


// void
// find_branch(Block_stmt const* s, Decl_set& branches)
// {
//   for (auto stmt : *s) {
//     find_branch(stmt, branches);
//   }
// }


// // This is the only real time that branches happen
// // the only things that cause branches are
// // 1. do expr
// // 2. flows goto
// void
// find_branch(Expr_stmt const* s, Decl_set& branches)
// {
//   if (is<Do_expr>(s->expr())) {
//     // look at the expression and extract the appropriate stage
//     Do_expr const* do_expr = cast<Do_expr>(s->expr());
//     // sanity check
//     lingo_assert(is<Id_expr>(do_expr->target()));
//     Decl const* target = as<Id_expr>(do_expr->target())->decl();

//     branches.insert(target);
//   }
// }


// void
// find_branch(Match_stmt const* s, Decl_set& branches)
// {
//   for (Stmt const* c : s->cases()) {
//     find_branch(as<Case_stmt>(c), branches);
//   }
// }


// void
// find_branch(Case_stmt const* s, Decl_set& branches)
// {
//   find_branch(s->stmt(), branches);
// }


// void
// find_branch(Flow_decl const* d, Decl_set& branches)
// {
//   find_branch(d->instructions(), branches);
// }


// void
// find_branch(Decl_stmt const* s, Decl_set& branches)
// {
//   // for now we will only handle flow declarations
//   // used in constant initialization of flow tables
//   // as being possible branches
//   if (is<Flow_decl>(s->decl()))
//     find_branch(cast<Flow_decl>(s->decl()), branches);
// }


// // This currently only works with decoders
// // Support for tables to come
// // Finding branches in tables is iffy
// void
// find_branch(Stmt const* s, Decl_set& branches)
// {
//   if (is<Block_stmt>(s))
//     find_branch(cast<Block_stmt>(s), branches);
//   if (is<Expr_stmt>(s))
//     find_branch(cast<Expr_stmt>(s), branches);
//   if (is<Match_stmt>(s))
//     find_branch(cast<Match_stmt>(s), branches);
//   if (is<Case_stmt>(s))
//     find_branch(cast<Case_stmt>(s), branches);
//   if (is<Decl_stmt>(s))
//     find_branch(cast<Decl_stmt>(s), branches);
// }


// // -------------------------------------------------------- //
// //      Depth-first search checks on each stage


// // FIXME: Figure out of decode declarations care about anything
// // prior. It doesn't seem to be the case now, but it may become an issue
// void
// check_stage(Decl const* d, Expr_seq const& requirements)
// {
//   for (auto e : requirements) {
//     auto search = cxt_bindings.find(as<Field_expr>(e)->name());
//     if (search == cxt_bindings.end()) {
//       error(d->location(), "Invalid field requirement. Field '{}' required but not decoded.", e);
//       error(d->location(), "Broken path: ");
//       for (auto stage : stack_) {
//         error(d->location(), "{} ->", stage->decl()->name());
//       }
//     }
//   }
// }


// // FIXME: PROVE THAT THIS ACTUALLY WORKS
// //
// // depth first search
// void
// dfs(Stage* s)
// {
//   s->visited = true;

//   // push all of its productions onto the bindings stack
//   for (auto p : s->productions())
//     cxt_bindings.insert(as<Field_expr>(p)->name());

//   // push the header name onto the bindings stack
//   cxt_bindings.insert(s->decl()->name());

//   // push stage onto stack for debugging purposes
//   stack_.push_back(s);

//   // check this stage
//   check_stage(s->decl(), s->requirements());

//   for (auto decl : s->branches()) {
//     if (decl != s->decl()) {
//       Stage* stage = pipeline.find(decl);
//       if (stage)
//         if (!stage->visited)
//           dfs(stage);
//     }
//   }

//   // cleanup
//   // pop off debugging stack
//   stack_.pop_back();

//   // pop all of the bindings off
//   for (auto p : s->productions())
//     cxt_bindings.erase(as<Field_expr>(p)->name());

//   // pop the header name off bindings stack
//   cxt_bindings.erase(s->decl()->name());

//   // unset the visited as you come back from recursion
//   // so that we can explore all possible paths instead of
//   // just one path
//   s->visited = false;
// }


// } // namespace



// void
// register_stage(Decode_decl const* d)
// {
//   lingo_assert(is<Block_stmt>(d->body()));

//   // Keep track of the possible branches in a decoder
//   // A branch happens anytime we encounter a do expression
//   Decl_set branches;

//   // Keep track of what fields each stage produces
//   Expr_seq product;

//   // bind the header decl into header environment
//   lingo_assert(is<Record_type>(d->header()));
//   Record_type const* htype = cast<Record_type>(d->header());
//   pipeline.env().headers().push(htype->decl()->name(), htype->decl());

//   // scan through the decode decl body
//   //    extract decls will add fields to the context
//   //    header type will be added to the context
//   for (auto stmt : *as<Block_stmt>(d->body())) {
//     find_productions(stmt, product);
//     find_branch(stmt, branches);
//   }

//   Stage* stage = stages_.make(d, branches, product);
//   pipeline.push_back(stage);

//   if (d->is_start()) {
//     if (!entry)
//       entry = stage;
//     else {
//       error("Multiple entry points found in pipeline.");
//       note("   First start: {}", entry->decl()->name());
//       note("   Second start: {}", d->name());
//       is_error_state = true;
//     }
//   }
// }


// void
// register_stage(Table_decl const* d)
// {
//   // Keep track of the possible branches in a decoder
//   // A branch happens anytime we encounter a do expression
//   Decl_set branches;

//   // Keep track of what fields each stage produces
//   Expr_seq product;

//   // check that the table had a default initialization
//   if (d->body().size() > 0) {
//     for (auto f : d->body()) {
//       // check for flow decl
//       lingo_assert(is<Flow_decl>(f));
//       Flow_decl const* flow = as<Flow_decl>(f);
//       find_branch(flow->instructions(), branches);
//     }
//   }

//   Stage* stage = stages_.make(d, branches, product);
//   pipeline.push_back(stage);

//   if (d->is_start()) {
//     if (!entry)
//       entry = stage;
//     else {
//       is_error_state = true;
//       error("Multiple entry points found in pipeline.");
//     }
//   }
// }


// // Checks the pipeline to confirm that it is well-formed
// // Note that many additional checks are made during
// // the lowering process. This check confirms the following:
// // 1. That all paths into tables from decoders are guaranteed
// //    to have extracted all fields required by the table at least once
// // 2. Tables do not form loops with goto statements
// // 3. Decoders never go back to a table that's been visited prior
// bool
// check_pipeline()
// {
//   // make sure there are actually
//   // components in the pipeline
//   if (!pipeline.size() > 0)
//     return false;

//   // we're going to do a depth first traversal
//   // until we hit a stage with no branches
//   // then we'll unwind and repeat until all branches
//   // have been visited
//   if (entry)
//     dfs(entry);
//   else {
//     error("No start declared for pipeline.");
//     return false;
//   }

//   return !is_error_state;
// }


// Value
// lookup_field_binding(String const* n)
// {
//   auto search = pipeline.env().fields().lookup(n);

//   if (search) {
//     return Value(search->count);
//   }
//   else
//     return Value(-1);
// }


// Value
// lookup_header_binding(String const* n)
// {
//   auto search = pipeline.env().headers().lookup(n);

//   if (search) {
//     return Value(search->count);
//   }
//   else
//     return Value(-1);
// }


// Pipeline const&
// get_pipeline()
// {
//   return pipeline;
// }


// int get_num_headers()
// {
//   return cxt_env.headers().size();
// }


// int get_num_fields()
// {
//   return cxt_env.fields().size();
// }


// // get the number mapped to the header within the
// // simplified environment
// Value_expr*
// get_header_binding(String const* s)
// {
//   return make_value_expr(get_int_type(), lookup_header_binding(s));
// }


// // get the number mapped to the field within the
// // simplified environment
// Value_expr*
// get_field_binding(String const* s)
// {
//   return make_value_expr(get_int_type(), lookup_field_binding(s));
// }


// Decl const*
// pipeline_get_start()
// {
//   if (entry) {
//     return entry->decl();
//   }

//   return nullptr;
// }

// } // namespace steve

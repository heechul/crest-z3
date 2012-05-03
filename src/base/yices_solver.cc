// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <queue>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <yices_c.h>


#include "base/yices_solver.h"

using std::make_pair;
using std::queue;
using std::set;

#define USE_Z3 1
#if USE_Z3
#  include <z3.h>
#endif

namespace crest {

typedef vector<const SymbolicPred*>::const_iterator PredIt;


bool YicesSolver::IncrementalSolve(const vector<value_t>& old_soln,
				   const map<var_t,type_t>& vars,
				   const vector<const SymbolicPred*>& constraints,
				   map<var_t,value_t>* soln) {
  set<var_t> tmp;
  typedef set<var_t>::const_iterator VarIt;

  // Build a graph on the variables, indicating a dependence when two
  // variables co-occur in a symbolic predicate.
  vector< set<var_t> > depends(vars.size());
  for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
    tmp.clear();
    (*i)->AppendVars(&tmp);
    for (VarIt j = tmp.begin(); j != tmp.end(); ++j) {
      depends[*j].insert(tmp.begin(), tmp.end());
    }
  }

  // Initialize the set of dependent variables to those in the constraints.
  // (Assumption: Last element of constraints is the only new constraint.)
  // Also, initialize the queue for the BFS.
  map<var_t,type_t> dependent_vars;
  queue<var_t> Q;
  tmp.clear();
  constraints.back()->AppendVars(&tmp);
  for (VarIt j = tmp.begin(); j != tmp.end(); ++j) {
    dependent_vars.insert(*vars.find(*j));
    Q.push(*j);
  }

  // Run the BFS.
  while (!Q.empty()) {
    var_t i = Q.front();
    Q.pop();
    for (VarIt j = depends[i].begin(); j != depends[i].end(); ++j) {
      if (dependent_vars.find(*j) == dependent_vars.end()) {
	Q.push(*j);
	dependent_vars.insert(*vars.find(*j));
      }
    }
  }

  // Generate the list of dependent constraints.
  vector<const SymbolicPred*> dependent_constraints;
  for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
    if ((*i)->DependsOn(dependent_vars))
      dependent_constraints.push_back(*i);
  }

  soln->clear();
  if (Solve(dependent_vars, dependent_constraints, soln)) {
    // Merge in the constrained variables.
    for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
      (*i)->AppendVars(&tmp);
    }
    for (set<var_t>::const_iterator i = tmp.begin(); i != tmp.end(); ++i) {
      if (soln->find(*i) == soln->end()) {
	soln->insert(make_pair(*i, old_soln[*i]));
      }
    }
    return true;
  }

  return false;
}


#if !USE_Z3
bool YicesSolver::Solve(const map<var_t,type_t>& vars,
			const vector<const SymbolicPred*>& constraints,
			map<var_t,value_t>* soln) {

  typedef map<var_t,type_t>::const_iterator VarIt;

  yices_enable_log_file("yices_log");
  yices_context ctx = yices_mk_context();
  assert(ctx);

  // Type limits.
  vector<yices_expr> min_expr(types::LONG_LONG+1);
  vector<yices_expr> max_expr(types::LONG_LONG+1);

  for (int i = types::U_CHAR; i <= types::LONG_LONG; i++) {
    min_expr[i] = yices_mk_num_from_string(ctx, const_cast<char*>(kMinValueStr[i]));
    max_expr[i] = yices_mk_num_from_string(ctx, const_cast<char*>(kMaxValueStr[i]));
    assert(min_expr[i]);
    assert(max_expr[i]);
  }

  char int_ty_name[] = "int";
  // fprintf(stderr, "yices_mk_mk_type(ctx, int_ty_name)\n");
  yices_type int_ty = yices_mk_type(ctx, int_ty_name);
  assert(int_ty);

  // Variable declarations.
  map<var_t,yices_var_decl> x_decl;
  map<var_t,yices_expr> x_expr;

  for (VarIt i = vars.begin(); i != vars.end(); ++i) {
    char buff[32];
    snprintf(buff, sizeof(buff), "x%d", i->first);
    // fprintf(stderr, "yices_mk_var_decl(ctx, buff, int_ty)\n");
    x_decl[i->first] = yices_mk_var_decl(ctx, buff, int_ty);
    // fprintf(stderr, "yices_mk_var_from_decl(ctx, x_decl[i->first])\n");
    x_expr[i->first] = yices_mk_var_from_decl(ctx, x_decl[i->first]);
    assert(x_decl[i->first]);
    assert(x_expr[i->first]);
    // fprintf(stderr, "yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]));
    // fprintf(stderr, "yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]));
  }

  // fprintf(stderr, "yices_mk_num(ctx, 0)\n");
  yices_expr zero = yices_mk_num(ctx, 0);
  assert(zero);

  { // Constraints.
    vector<yices_expr> terms;

    for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
      const SymbolicExpr& se = (*i)->expr();
#if 1
      string s = "";
      se.AppendToString(&s);
      fprintf(stderr, "%s ", s.c_str());
#endif
      terms.clear();
      terms.push_back(yices_mk_num(ctx, se.const_term()));

      for (SymbolicExpr::TermIt j = se.terms().begin(); j != se.terms().end(); ++j) {
	yices_expr prod[2] = { x_expr[j->first], yices_mk_num(ctx, j->second) };
	terms.push_back(yices_mk_mul(ctx, prod, 2));
      }
      yices_expr e = yices_mk_sum(ctx, &terms.front(), terms.size());
      
      yices_expr pred;
      switch((*i)->op()) {
      case ops::EQ:  
	pred = yices_mk_eq(ctx, e, zero); 
	fprintf(stderr, "==\n");
	break;
      case ops::NEQ: 
	pred = yices_mk_diseq(ctx, e, zero); 
	fprintf(stderr, "!=\n");
	break;
      case ops::GT:  
	pred = yices_mk_gt(ctx, e, zero); 
	fprintf(stderr, ">\n");
	break;
      case ops::LE:  
	pred = yices_mk_le(ctx, e, zero); 
	fprintf(stderr, "<=\n");
	break;
      case ops::LT:  
	pred = yices_mk_lt(ctx, e, zero); 
	fprintf(stderr, "<\n");
	break;
      case ops::GE:  
	pred = yices_mk_ge(ctx, e, zero); 
	fprintf(stderr, ">=\n");
	break;
      default:
	fprintf(stderr, "Unknown comparison operator: %d\n", (*i)->op());
	exit(1);
      }
      yices_assert(ctx, pred);
    }
  }

  bool success = (yices_check(ctx) == l_true);
  if (success) {
    soln->clear();
    yices_model model = yices_get_model(ctx);

    if (model == NULL) {
      fprintf(stderr, "ERR: Can't get model...\n");
      goto out;
    }

    for (VarIt i = vars.begin(); i != vars.end(); ++i) {
      long val;
      assert(yices_get_int_value(model, x_decl[i->first], &val));
      soln->insert(make_pair(i->first, val));
    }
  }
 out:

  yices_del_context(ctx);
  return success;
}
#else /* USE_Z3 */

/**
   \brief exit gracefully in case of error.
*/
void exitf(const char* message) 
{
  fprintf(stderr,"BUG: %s.\n", message);
  exit(1);
}

/**
   \brief Simpler error handler.
 */
void error_handler(Z3_error_code e) 
{
  printf("Error code: %d\n", e);
  exitf("incorrect use of Z3");
}

/**
   \brief exit if unreachable code was reached.
*/
void unreachable() 
{
    exitf("unreachable code was reached");
}

/**
   \brief Create a logical context.  

   Enable model construction. Other configuration parameters can be passed in the cfg variable.

   Also enable tracing to stderr and register custom error handler.
*/
static Z3_context mk_context_custom(Z3_config cfg, Z3_error_handler err) 
{
    Z3_context ctx;
    
    Z3_set_param_value(cfg, "MODEL", "true");
    ctx = Z3_mk_context(cfg);
#ifdef TRACING
    Z3_trace_to_stderr(ctx);
#endif
    Z3_set_error_handler(ctx, err);
    
    return ctx;
}

/**
   \brief Create a logical context.

   Enable model construction only.

   Also enable tracing to stderr and register standard error handler.
*/
static Z3_context mk_context() 
{
    Z3_config  cfg;
    Z3_context ctx;
    cfg = Z3_mk_config();
    ctx = mk_context_custom(cfg, error_handler);
    Z3_del_config(cfg);
    return ctx;
}

/**
   \brief Create a variable using the given name and type.
*/
Z3_ast mk_var(Z3_context ctx, const char * name, Z3_sort ty) 
{
    Z3_symbol   s  = Z3_mk_string_symbol(ctx, name);
    return Z3_mk_const(ctx, s, ty);
}

/**
   \brief Create an integer variable using the given name.
*/
Z3_ast mk_int_var(Z3_context ctx, const char * name) 
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return mk_var(ctx, name, ty);
}

/**
   \brief Create a Z3 integer node using a C int. 
*/
Z3_ast mk_int(Z3_context ctx, int v) 
{
    Z3_sort ty = Z3_mk_int_sort(ctx);
    return Z3_mk_int(ctx, v, ty);
}


/**
   \brief Check whether the logical context is satisfiable, and compare the result with the expected result.
   If the context is satisfiable, then display the model.
*/
void check(Z3_context ctx, Z3_lbool expected_result)
{
    Z3_model m      = 0;
    Z3_lbool result = Z3_check_and_get_model(ctx, &m);
    switch (result) {
    case Z3_L_FALSE:
        printf("unsat\n");
        break;
    case Z3_L_UNDEF:
        printf("unknown\n");
        printf("potential model:\n%s\n", Z3_model_to_string(ctx, m));
        break;
    case Z3_L_TRUE:
        printf("sat\n%s\n", Z3_model_to_string(ctx, m));
        break;
    }
    if (m) {
        Z3_del_model(ctx, m);
    }
    if (result != expected_result) {
        exitf("unexpected result");
    }
}

/**
   \brief Display a symbol in the given output stream.
*/
void display_symbol(Z3_context c, FILE * out, Z3_symbol s) 
{
    switch (Z3_get_symbol_kind(c, s)) {
    case Z3_INT_SYMBOL:
        fprintf(out, "#%d", Z3_get_symbol_int(c, s));
        break;
    case Z3_STRING_SYMBOL:
        fprintf(out, Z3_get_symbol_string(c, s));
        break;
    default:
        unreachable();
    }
}

/**
   \brief Display the given type.
*/
void display_sort(Z3_context c, FILE * out, Z3_sort ty) 
{
    switch (Z3_get_sort_kind(c, ty)) {
    case Z3_UNINTERPRETED_SORT:
        display_symbol(c, out, Z3_get_sort_name(c, ty));
        break;
    case Z3_BOOL_SORT:
        fprintf(out, "bool");
        break;
    case Z3_INT_SORT:
        fprintf(out, "int");
        break;
    case Z3_REAL_SORT:
        fprintf(out, "real");
        break;
    case Z3_BV_SORT:
        fprintf(out, "bv%d", Z3_get_bv_sort_size(c, ty));
        break;
    case Z3_ARRAY_SORT: 
        fprintf(out, "[");
        display_sort(c, out, Z3_get_array_sort_domain(c, ty));
        fprintf(out, "->");
        display_sort(c, out, Z3_get_array_sort_range(c, ty));
        fprintf(out, "]");
        break;
    case Z3_DATATYPE_SORT:
		if (Z3_get_datatype_sort_num_constructors(c, ty) != 1) 
		{
			fprintf(out, "%s", Z3_sort_to_string(c,ty));
			break;
		}
		{
        unsigned num_fields = Z3_get_tuple_sort_num_fields(c, ty);
        unsigned i;
        fprintf(out, "(");
        for (i = 0; i < num_fields; i++) {
            Z3_func_decl field = Z3_get_tuple_sort_field_decl(c, ty, i);
            if (i > 0) {
                fprintf(out, ", ");
            }
            display_sort(c, out, Z3_get_range(c, field));
        }
        fprintf(out, ")");
        break;
    }
    default:
        fprintf(out, "unknown[");
        display_symbol(c, out, Z3_get_sort_name(c, ty));
        fprintf(out, "]");
        break;
    }
}

/**
   \brief Custom ast pretty printer. 

   This function demonstrates how to use the API to navigate terms.
*/
void display_ast(Z3_context c, FILE * out, Z3_ast v) 
{
    switch (Z3_get_ast_kind(c, v)) {
    case Z3_NUMERAL_AST: {
        Z3_sort t;
        fprintf(out, Z3_get_numeral_string(c, v));
        t = Z3_get_sort(c, v);
        fprintf(out, ":");
        display_sort(c, out, t);
        break;
    }
    case Z3_APP_AST: {
        unsigned i;
        Z3_app app = Z3_to_app(c, v);
        unsigned num_fields = Z3_get_app_num_args(c, app);
        Z3_func_decl d = Z3_get_app_decl(c, app);
        fprintf(out, Z3_func_decl_to_string(c, d));
        if (num_fields > 0) {
            fprintf(out, "[");
            for (i = 0; i < num_fields; i++) {
                if (i > 0) {
                    fprintf(out, ", ");
                }
                display_ast(c, out, Z3_get_app_arg(c, app, i));
            }
            fprintf(out, "]");
        }
        break;
    }
    case Z3_QUANTIFIER_AST: {
        fprintf(out, "quantifier");
        ;	
    }
    default:
        fprintf(out, "#unknown");
    }
}


bool YicesSolver::Solve(const map<var_t,type_t>& vars,
			const vector<const SymbolicPred*>& constraints,
			map<var_t,value_t>* soln) {

  typedef map<var_t,type_t>::const_iterator VarIt;

  yices_enable_log_file("yices_log");
  yices_context ctx = yices_mk_context();

  Z3_context ctx_z3 = mk_context();
  Z3_sort int_ty_z3 = Z3_mk_int_sort(ctx_z3);

  assert(ctx);

  // Type limits.
  vector<yices_expr> min_expr(types::LONG_LONG+1);
  vector<yices_expr> max_expr(types::LONG_LONG+1);

  vector<Z3_ast> min_expr_z3(types::LONG_LONG+1);
  vector<Z3_ast> max_expr_z3(types::LONG_LONG+1);

  for (int i = types::U_CHAR; i <= types::LONG_LONG; i++) {
    min_expr[i] = yices_mk_num_from_string(ctx, const_cast<char*>(kMinValueStr[i]));
    max_expr[i] = yices_mk_num_from_string(ctx, const_cast<char*>(kMaxValueStr[i]));

    min_expr_z3[i] = Z3_mk_numeral(ctx_z3, const_cast<char*>(kMinValueStr[i]), int_ty_z3);
    max_expr_z3[i] = Z3_mk_numeral(ctx_z3, const_cast<char*>(kMaxValueStr[i]), int_ty_z3);
    
    assert(min_expr[i]);
    assert(max_expr[i]);
  }

  char int_ty_name[] = "int";
  // fprintf(stderr, "yices_mk_mk_type(ctx, int_ty_name)\n");
  yices_type int_ty = yices_mk_type(ctx, int_ty_name);

  assert(int_ty);

  // Variable declarations.
  map<var_t,yices_var_decl> x_decl;
  map<var_t,yices_expr> x_expr;

  map<var_t,Z3_ast> x_expr_z3;

  for (VarIt i = vars.begin(); i != vars.end(); ++i) {
    char buff[32];
    snprintf(buff, sizeof(buff), "x%d", i->first);

    // fprintf(stderr, "yices_mk_var_decl(ctx, buff, int_ty)\n");
    x_decl[i->first] = yices_mk_var_decl(ctx, buff, int_ty);
    // fprintf(stderr, "yices_mk_var_from_decl(ctx, x_decl[i->first])\n");
    x_expr[i->first] = yices_mk_var_from_decl(ctx, x_decl[i->first]);
    assert(x_decl[i->first]);
    assert(x_expr[i->first]);
    // fprintf(stderr, "yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]));
    // fprintf(stderr, "yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]));

    x_expr_z3[i->first] = mk_var(ctx_z3, buff, int_ty_z3);
    Z3_assert_cnstr(ctx_z3, Z3_mk_ge(ctx_z3, x_expr_z3[i->first], min_expr_z3[i->second]));
    Z3_assert_cnstr(ctx_z3, Z3_mk_le(ctx_z3, x_expr_z3[i->first], max_expr_z3[i->second]));
  }

  // fprintf(stderr, "yices_mk_num(ctx, 0)\n");
  yices_expr zero = yices_mk_num(ctx, 0);

  Z3_ast zero_z3 = mk_int(ctx_z3, 0);

  assert(zero);
  assert(zero_z3);

  { // Constraints.
    vector<yices_expr> terms;
    vector<Z3_ast> terms_z3;
    for (PredIt i = constraints.begin(); i != constraints.end(); ++i) {
      const SymbolicExpr& se = (*i)->expr();
#if 1
      string s = "";
      se.AppendToString(&s);
      fprintf(stderr, "%s ", s.c_str());
#endif
      terms.clear();
      terms.push_back(yices_mk_num(ctx, se.const_term()));

      terms_z3.clear();
      terms_z3.push_back(mk_int(ctx_z3, se.const_term()));

      for (SymbolicExpr::TermIt j = se.terms().begin(); j != se.terms().end(); ++j) {
	yices_expr prod[2] = { x_expr[j->first], yices_mk_num(ctx, j->second) };
	terms.push_back(yices_mk_mul(ctx, prod, 2));

	Z3_ast prod_z3[2] = { x_expr_z3[j->first], mk_int(ctx_z3, j->second)};
	terms_z3.push_back(Z3_mk_mul(ctx_z3, 2, prod_z3));
      }
      yices_expr e = yices_mk_sum(ctx, &terms.front(), terms.size());
      yices_expr pred;

      Z3_ast e_z3 = Z3_mk_add(ctx_z3, terms_z3.size(), &terms_z3.front());
      Z3_ast pred_z3;

      switch((*i)->op()) {
      case ops::EQ:  
	pred = yices_mk_eq(ctx, e, zero); 
	pred_z3 = Z3_mk_eq(ctx_z3, e_z3, zero_z3);
	fprintf(stderr, "==\n");
	break;
      case ops::NEQ: 
	fprintf(stderr, "!=\n");
	pred = yices_mk_diseq(ctx, e, zero); 
	// fprintf(stderr, "neq: ast %s\n", Z3_ast_to_string(ctx_z3, e_z3));
	// fprintf(stderr, "neq: sort %s\n", Z3_sort_to_string(ctx_z3, int_ty_z3));
	pred_z3 = Z3_mk_not(ctx_z3, Z3_mk_eq(ctx_z3, e_z3, zero_z3));
	break;
      case ops::GT:  
	pred = yices_mk_gt(ctx, e, zero); 
	pred_z3 = Z3_mk_gt(ctx_z3, e_z3, zero_z3);
	fprintf(stderr, ">\n");
	break;
      case ops::LE:  
	pred = yices_mk_le(ctx, e, zero); 
	pred_z3 = Z3_mk_le(ctx_z3, e_z3, zero_z3);
	fprintf(stderr, "<=\n");
	break;
      case ops::LT:  
	pred = yices_mk_lt(ctx, e, zero); 
	pred_z3 = Z3_mk_lt(ctx_z3, e_z3, zero_z3);
	fprintf(stderr, "<\n");
	break;
      case ops::GE:  
	pred = yices_mk_ge(ctx, e, zero); 
	pred_z3 = Z3_mk_ge(ctx_z3, e_z3, zero_z3);
	fprintf(stderr, ">=\n");
	break;
      default:
	fprintf(stderr, "Unknown comparison operator: %d\n", (*i)->op());
	exit(1);
      }
      yices_assert(ctx, pred);
      Z3_assert_cnstr(ctx_z3, pred_z3);
    }
  }

  bool success = (yices_check(ctx) == l_true);

  Z3_model model_z3 = 0;
  Z3_lbool success_z3 = Z3_check_and_get_model(ctx_z3, &model_z3);

  if (success != (success_z3 == Z3_L_TRUE)) {
    fprintf(stderr, "ERR: Two are different. yices=%d, z3=%d\n\n\n", 
	    success, success_z3);
  }

  if (success) {
    soln->clear();
    yices_model model = yices_get_model(ctx);
    
    if (model == NULL) {
      fprintf(stderr, "ERR: Can't get model...\n");
      goto out;
    }
    for (VarIt i = vars.begin(); i != vars.end(); ++i) {
      long val;
      assert(yices_get_int_value(model, x_decl[i->first], &val));
      
      // soln->insert(make_pair(i->first, val));

      fprintf(stderr, "yices: insert x%d %d\n", i->first, val);
    }
  }

  if (success_z3 == Z3_L_TRUE) {
    int num_constraints = Z3_get_model_num_constants(ctx_z3, model_z3);
    for (int i = 0; i < num_constraints; i++) {
        Z3_symbol name;
        Z3_func_decl cnst = Z3_get_model_constant(ctx_z3, model_z3, i);
        Z3_ast a, v;
        Z3_bool ok;
        name = Z3_get_decl_name(ctx_z3, cnst);
        a = Z3_mk_app(ctx_z3, cnst, 0, 0);
        v = a;
        ok = Z3_eval(ctx_z3, model_z3, a, &v);
	int idx;
	sscanf(Z3_get_symbol_string(ctx_z3, name), "x%d", &idx);
	long val = strtol(Z3_get_numeral_string(ctx_z3, v), NULL, 0);
	fprintf(stderr, "%s %s | x%d %ld\n", 
		Z3_get_symbol_string(ctx_z3, name),
		Z3_get_numeral_string(ctx_z3, v), 
		idx, val);
	soln->insert(make_pair(idx, val));
    }
  }


 out:
  yices_del_context(ctx);

  Z3_del_context(ctx_z3);

  return success;
}
#endif /* USE_Z3 */

}  // namespace crest


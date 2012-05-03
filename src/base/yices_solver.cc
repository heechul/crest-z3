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
  fprintf(stderr, "yices_mk_mk_type(ctx, int_ty_name)\n");
  yices_type int_ty = yices_mk_type(ctx, int_ty_name);
  assert(int_ty);

  // Variable declarations.
  map<var_t,yices_var_decl> x_decl;
  map<var_t,yices_expr> x_expr;

  for (VarIt i = vars.begin(); i != vars.end(); ++i) {
    char buff[32];
    snprintf(buff, sizeof(buff), "x%d", i->first);
    fprintf(stderr, "yices_mk_var_decl(ctx, buff, int_ty)\n");
    x_decl[i->first] = yices_mk_var_decl(ctx, buff, int_ty);
    fprintf(stderr, "yices_mk_var_from_decl(ctx, x_decl[i->first])\n");
    x_expr[i->first] = yices_mk_var_from_decl(ctx, x_decl[i->first]);
    assert(x_decl[i->first]);
    assert(x_expr[i->first]);
    fprintf(stderr, "yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]));
    fprintf(stderr, "yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]));
  }

  fprintf(stderr, "yices_mk_num(ctx, 0)\n");
  yices_expr zero = yices_mk_num(ctx, 0);
  assert(zero);

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
    for (VarIt i = vars.begin(); i != vars.end(); ++i) {
      long val;
      assert(yices_get_int_value(model, x_decl[i->first], &val));
      soln->insert(make_pair(i->first, val));
    }
  }

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
  fprintf(stderr, "yices_mk_mk_type(ctx, int_ty_name)\n");
  yices_type int_ty = yices_mk_type(ctx, int_ty_name);

  assert(int_ty);

  // Variable declarations.
  map<var_t,yices_var_decl> x_decl;
  map<var_t,yices_expr> x_expr;

  map<var_t,Z3_ast> x_expr_z3;

  for (VarIt i = vars.begin(); i != vars.end(); ++i) {
    char buff[32];
    snprintf(buff, sizeof(buff), "x%d", i->first);

    fprintf(stderr, "yices_mk_var_decl(ctx, buff, int_ty)\n");
    x_decl[i->first] = yices_mk_var_decl(ctx, buff, int_ty);
    fprintf(stderr, "yices_mk_var_from_decl(ctx, x_decl[i->first])\n");
    x_expr[i->first] = yices_mk_var_from_decl(ctx, x_decl[i->first]);
    assert(x_decl[i->first]);
    assert(x_expr[i->first]);
    fprintf(stderr, "yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_ge(ctx, x_expr[i->first], min_expr[i->second]));
    fprintf(stderr, "yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]))\n");
    yices_assert(ctx, yices_mk_le(ctx, x_expr[i->first], max_expr[i->second]));

    x_expr_z3[i->first] = mk_var(ctx_z3, buff, int_ty_z3);
    Z3_assert_cnstr(ctx_z3, Z3_mk_ge(ctx_z3, x_expr_z3[i->first], min_expr_z3[i->second]));
    Z3_assert_cnstr(ctx_z3, Z3_mk_le(ctx_z3, x_expr_z3[i->first], max_expr_z3[i->second]));
  }

  fprintf(stderr, "yices_mk_num(ctx, 0)\n");
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

      Z3_ast e_z3 = Z3_mk_add(ctx_z3, terms_z3.size(), &terms_z3.front());

      yices_expr pred;
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

  check(ctx_z3, (success) ? Z3_L_TRUE : Z3_L_FALSE);

  if (success) {
    soln->clear();
    yices_model model = yices_get_model(ctx);
    for (VarIt i = vars.begin(); i != vars.end(); ++i) {
      long val;
      assert(yices_get_int_value(model, x_decl[i->first], &val));
      soln->insert(make_pair(i->first, val));
    }
  }

  yices_del_context(ctx);

  Z3_del_context(ctx_z3);

  return success;
}
#endif /* USE_Z3 */

}  // namespace crest


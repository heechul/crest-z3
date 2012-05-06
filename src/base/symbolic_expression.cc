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
#include <stdio.h>
#include "base/symbolic_expression.h"

#define DEBUG(x) 
#define MAX_LINE_BUF 1024

namespace crest {

typedef map<var_t,value_t>::iterator It;
typedef map<var_t,value_t>::const_iterator ConstIt;


SymbolicExpr::~SymbolicExpr() { }

SymbolicExpr::SymbolicExpr() : const_(0) { }

SymbolicExpr::SymbolicExpr(value_t c) : const_(c) { }

SymbolicExpr::SymbolicExpr(value_t c, var_t v) : const_(0) {
  coeff_[v] = c;

  char tmp[128];
  sprintf(tmp, "x%d", v);
  expr_str_ = tmp;
}

SymbolicExpr::SymbolicExpr(const SymbolicExpr& e)
  : const_(e.const_), coeff_(e.coeff_), expr_str_(e.expr_str_) { }


void SymbolicExpr::Negate() {
  const_ = -const_;
  expr_str_ = "( - 0 " + expr_str_ + ")";
}


void SymbolicExpr::AppendVars(set<var_t>* vars) const {
  for (ConstIt i = coeff_.begin(); i != coeff_.end(); ++i) {
    vars->insert(i->first);
  }
}

bool SymbolicExpr::DependsOn(const map<var_t,type_t>& vars) const {
  for (ConstIt i = coeff_.begin(); i != coeff_.end(); ++i) {
    if (vars.find(i->first) != vars.end())
      return true;
  }
  return false;
}


void SymbolicExpr::AppendToString(string* s) const {
  s->append(expr_str_);
}


void SymbolicExpr::Serialize(string* s) const {
  /* write constant value */
  s->append( expr_str_ + "\n");
  DEBUG(fprintf(stderr, "%s: %s\n", __FUNCTION__, expr_str_.c_str() ));
}


bool SymbolicExpr::Parse(istream& s) {
  char buf[MAX_LINE_BUF];

  s.getline(buf, MAX_LINE_BUF);
  expr_str_ = string(buf);
  DEBUG(fprintf(stderr, "%s: %s\n", __FUNCTION__, expr_str_.c_str() ));

  for (int i = 0; i < MAX_LINE_BUF && buf[i] != '\n'; i++) {
    if (buf[i] == 'x') {
      int var;
      sscanf(&buf[i+1], "%d", &var);
      if (var >= 0 && var < 128) /* valid variable range */
	coeff_[var] = 1;
    }
  }
  DEBUG(fprintf(stderr, "%s: #vars=%d\n", __FUNCTION__, coeff_.size()));

  return !s.fail();
}


const SymbolicExpr& SymbolicExpr::operator+=(const SymbolicExpr& e) {
  const_ += e.const_;

  for (ConstIt i = e.coeff_.begin(); i != e.coeff_.end(); ++i) {
    It j = coeff_.find(i->first);
    if (j == coeff_.end()) {
      coeff_[i->first] = 1;
    }
  }
  expr_str_ = "(+ " + expr_str_ + " " + e.expr_str_ + " )";
  return *this;
}


const SymbolicExpr& SymbolicExpr::operator-=(const SymbolicExpr& e) {
  const_ -= e.const_;

  for (ConstIt i = e.coeff_.begin(); i != e.coeff_.end(); ++i) {
    It j = coeff_.find(i->first);
    if (j == coeff_.end()) {
      coeff_[i->first] = 1;
    }
  }
  expr_str_ = "(- " + expr_str_ + " " + e.expr_str_ + " )";
  return *this;
}

const SymbolicExpr& SymbolicExpr::operator*=(const SymbolicExpr & e) {
  const_ *= e.const_;

  for (ConstIt i = e.coeff_.begin(); i != e.coeff_.end(); ++i) {
    It j = coeff_.find(i->first);
    if (j == coeff_.end()) {
      coeff_[i->first] = 1;
    }
  }

  expr_str_ = "(* " + expr_str_ + " " + e.expr_str_ + " )";
  return *this;
}


const SymbolicExpr& SymbolicExpr::operator/=(const SymbolicExpr & e) {
  const_ *= e.const_;

  for (ConstIt i = e.coeff_.begin(); i != e.coeff_.end(); ++i) {
    It j = coeff_.find(i->first);
    if (j == coeff_.end()) {
      coeff_[i->first] = 1;
    }
  }

  expr_str_ = "(div " + expr_str_ + " " + e.expr_str_ + " )";
  return *this;
}


const SymbolicExpr& SymbolicExpr::operator+=(value_t c) {
  const_ += c;

  char buf[32];
  sprintf(buf, "%lld", c);
  expr_str_ = "(+ " + expr_str_ + " " + string(buf) + " )";
  return *this;
}


const SymbolicExpr& SymbolicExpr::operator-=(value_t c) {
  const_ -= c;

  char buf[32];
  sprintf(buf, "%lld", c);
  expr_str_ = "(- " + expr_str_ + " " + string(buf) + " )";

  return *this;
}


const SymbolicExpr& SymbolicExpr::operator*=(value_t c) {
  if (c == 0) {
    coeff_.clear();
    const_ = 0;
    expr_str_ = "";
    return *this;
  } 

  char buf[32];
  sprintf(buf, "%lld", c);
  expr_str_ = "(* " + expr_str_ + " " + string(buf) + " )";

  return *this;
}

const SymbolicExpr& SymbolicExpr::operator/=(value_t c) {

  assert(c != 0);

  char buf[32];
  sprintf(buf, "%lld", c);
  expr_str_ = "(div " + expr_str_ + " " + string(buf) + " )";

  return *this;
}

const SymbolicExpr& SymbolicExpr::operator%=(value_t c) {

  assert(c != 0);

  char buf[32];
  sprintf(buf, "%lld", c);
  expr_str_ = "(mod " + expr_str_ + " " + string(buf) + " )";

  return *this;
}


bool SymbolicExpr::operator==(const SymbolicExpr& e) const {
  return ((const_ == e.const_) && (coeff_ == e.coeff_));
}


}  // namespace crest

// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "base/symbolic_predicate.h"

#include <stdio.h>

#define DEBUG(x) 

namespace crest {

SymbolicPred::SymbolicPred()
  : op_(ops::EQ), expr_(new SymbolicExpr(0)) { }

SymbolicPred::SymbolicPred(compare_op_t op, SymbolicExpr* expr)
  : op_(op), expr_(expr) { 
}

SymbolicPred::~SymbolicPred() {
  delete expr_;
}

void SymbolicPred::Negate() {
  op_ = NegateCompareOp(op_);
}

void SymbolicPred::AppendToString(string* s) const {
  const char* symbol[] = { "= ", "/=", "> ", "<=", "< ", ">=" };

  /* for Z3 which does not support /= */
  if (op_ == ops::NEQ)
    s->append( "(not (=  " + expr_->get_expr_str() + " 0 ) )");
  else 
    s->append( "(" + string(symbol[op_]) + " " + expr_->get_expr_str() + " 0 )");
}

void SymbolicPred::Serialize(string* s) const {
  char buf[32];
  sprintf(buf, "%d\n", op_);
  s->append(string(buf));
  DEBUG(fprintf(stderr, "%s: op_=%d\n", __FUNCTION__, op_));
  expr_->Serialize(s);
}

bool SymbolicPred::Parse(istream& s) {
  char buf[8];
  s.getline(buf, 8);
  sscanf(buf, "%d", &op_);
  DEBUG(fprintf(stderr, "%s: op_=%d\n", __FUNCTION__, op_));
  expr_->Parse(s);
}

bool SymbolicPred::Equal(const SymbolicPred& p) const {
  return ((op_ == p.op_) && (*expr_ == *p.expr_));
}


}  // namespace crest

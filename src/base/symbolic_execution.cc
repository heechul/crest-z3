// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <utility>
#include <stdio.h>

#include "base/symbolic_execution.h"

#define DEBUG(x)
#define MAX_LINE_BUF 1024

namespace crest {

SymbolicExecution::SymbolicExecution() { }

SymbolicExecution::SymbolicExecution(bool pre_allocate)
  : path_(pre_allocate) { }

SymbolicExecution::~SymbolicExecution() { }

void SymbolicExecution::Swap(SymbolicExecution& se) {
  vars_.swap(se.vars_);
  inputs_.swap(se.inputs_);
  path_.Swap(se.path_);
}

void SymbolicExecution::Serialize(string* s) const {
  typedef map<var_t,type_t>::const_iterator VarIt;
  char buf[32];
  size_t len = vars_.size();

  /* #vars */
  sprintf(buf, "%d\n", len); // # of variables
  s->append(string(buf));

  /* var_type var_value */
  for (VarIt i = vars_.begin(); i != vars_.end(); ++i) {
    sprintf(buf, "%d %lld\n", i->second, inputs_[i->first]); /* type, value */
    s->append(string(buf));
  }

  /* path */
  path_.Serialize(s);
}

bool SymbolicExecution::Parse(istream& s) {
  // Read the inputs.
  size_t len;
  char buf[MAX_LINE_BUF];

  s.getline(buf, MAX_LINE_BUF);
  sscanf(buf, "%d\n", &len);

  DEBUG(fprintf(stderr, "%s: #vars = %d\n", __FUNCTION__, len));

  vars_.clear();
  inputs_.resize(len);

  for (size_t i = 0; i < len; i++) {
    int type;
    long long int value;
    s.getline(buf, MAX_LINE_BUF);
    sscanf(buf, "%d %lld\n", &type, &value);

    vars_[i] = (type_t)type; /* var type */
    inputs_[i] = value; /* var value */

    DEBUG(fprintf(stderr, "%s: var%d: type=%d, value=%lld\n", 
		  __FUNCTION__, i, type, value));
  }

  // Write the path.
  return (path_.Parse(s) && !s.fail());
}

}  // namespace crest

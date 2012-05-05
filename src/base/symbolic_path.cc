// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CREST, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include "base/symbolic_path.h"
#include <stdio.h>

#define DEBUG(x)

#define MAX_LINE_BUF 4096


namespace crest {

SymbolicPath::SymbolicPath() { }

SymbolicPath::SymbolicPath(bool pre_allocate) {
  if (pre_allocate) {
    // To cut down on re-allocation.
    branches_.reserve(4000000);
    constraints_idx_.reserve(50000);
    constraints_.reserve(50000);
  }
}

SymbolicPath::~SymbolicPath() {
  for (size_t i = 0; i < constraints_.size(); i++)
    delete constraints_[i];
}

void SymbolicPath::Swap(SymbolicPath& sp) {
  branches_.swap(sp.branches_);
  constraints_idx_.swap(sp.constraints_idx_);
  constraints_.swap(sp.constraints_);
}

void SymbolicPath::Push(branch_id_t bid) {
  branches_.push_back(bid);
}

void SymbolicPath::Push(branch_id_t bid, SymbolicPred* constraint) {
  if (constraint) {
    constraints_.push_back(constraint);
    constraints_idx_.push_back(branches_.size());
  }
  branches_.push_back(bid);
}

void SymbolicPath::Serialize(string* s) const {
  typedef vector<SymbolicPred*>::const_iterator ConIt;
  typedef vector<size_t*>::const_iterator ConIdxIt;
  typedef vector<branch_id_t*>::const_iterator BranIt;

  char buf[32];

  // Write the path.
  size_t len = branches_.size();

  sprintf(buf, "%d\n", len);
  s->append(string(buf));

  for (size_t i = 0; i < len; i++) {
    sprintf(buf, "%d ", branches_[i]);
    s->append(string(buf));
  }
  s->append(string("\n"));

  // Write the path constraints.
  len = constraints_.size();
  sprintf(buf, "%d\n", len);
  s->append(string(buf));
  for (size_t i = 0; i < len; i++) {
    sprintf(buf, "%d ", constraints_idx_[i]);
    s->append(string(buf));
  }
  s->append(string("\n"));

  // write constraints
  for (ConIt i = constraints_.begin(); i != constraints_.end(); ++i) {
    (*i)->Serialize(s);
  }
}

bool SymbolicPath::Parse(istream& s) {
  typedef vector<SymbolicPred*>::iterator ConIt;
  typedef vector<size_t*>::iterator ConIdxIt;
  typedef vector<branch_id_t*>::iterator BranIt;
  size_t len;
  char buf[MAX_LINE_BUF];

  // read #branches
  s.getline(buf, MAX_LINE_BUF);
  sscanf(buf, "%d", &len);

  DEBUG(fprintf(stderr, "#branches = %d\n", len));
  DEBUG(fprintf(stderr, "branch ids:\n"));  
  branches_.resize(len);
  s.getline(buf, MAX_LINE_BUF);
  char *ptr = buf;
  for(size_t i = 0; i < len; i++) {
    int bid;
    sscanf(ptr, "%d", &bid);
    DEBUG(fprintf(stderr, "%d ", bid));
    while (*ptr != ' ') ptr++;
    ptr++;
    branches_.push_back(bid);
  }
  DEBUG(fprintf(stderr, "\n"));
  if (s.fail())
    return false;

  // Clean up any existing path constraints.
  for (size_t i = 0; i < constraints_.size(); i++)
    delete constraints_[i];

  // Read the path constraints.
  s.getline(buf, MAX_LINE_BUF);
  sscanf(buf, "%d", &len);
  constraints_idx_.resize(len);
  constraints_.resize(len);

  DEBUG(fprintf(stderr, "#constaints = %d\n", len));
  DEBUG(fprintf(stderr, "constraints idxes:\n"));  

  s.getline(buf, MAX_LINE_BUF);
  ptr = buf;
  for(size_t i = 0; i < len; i++) {
    int cid;
    sscanf(ptr, "%d", &cid);
    DEBUG(fprintf(stderr, "%d ", cid));
    while (*ptr != ' ') ptr++;
    ptr++;
    constraints_idx_.push_back(cid);
  }
  DEBUG(fprintf(stderr, "\n"));

  if (s.fail())
    return false;

  DEBUG(fprintf(stderr, "Parse predicates\n"));
  for (ConIt i = constraints_.begin(); i != constraints_.end(); ++i) {
    *i = new SymbolicPred();
    if (!(*i)->Parse(s))
      return false;
  }
  return !s.fail();
}

}  // namespace crest

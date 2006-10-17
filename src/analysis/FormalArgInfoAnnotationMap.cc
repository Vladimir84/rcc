// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: FormalArgInfoAnnotationMap.cc
//
// Maps formal arguments of a function to annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>

#include "FormalArgInfoAnnotationMap.h"

namespace RAnnot {

// type definitions for readability
typedef FormalArgInfoAnnotationMap::MyKeyT MyKeyT;
typedef FormalArgInfoAnnotationMap::MyMappedT MyMappedT;
typedef FormalArgInfoAnnotationMap::iterator iterator;
typedef FormalArgInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor -----

FormalArgInfoAnnotationMap::FormalArgInfoAnnotationMap(bool ownsAnnotations /* = true */)
: m_computed(false),
  m_map()
  {}

FormalArgInfoAnnotationMap::~FormalArgInfoAnnotationMap() {}

// ----- singleton pattern -----

FormalArgInfoAnnotationMap * FormalArgInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT FormalArgInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void FormalArgInfoAnnotationMap::create() {
  m_instance = new FormalArgInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

// ----- demand-driven analysis -----

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & FormalArgInfoAnnotationMap::operator[](const MyKeyT & k) {
  return m_map[k];
}

// Perform the computation if necessary and return the requested data.
AnnotationMap::MyMappedT FormalArgInfoAnnotationMap::get(const AnnotationMap::MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in map");
  }

  return annot->second;
}
  
bool FormalArgInfoAnnotationMap::is_computed() {
  return m_computed;
}

// ----- iterators -----

iterator FormalArgInfoAnnotationMap::begin() { return m_map.begin(); }
const_iterator  FormalArgInfoAnnotationMap::begin() const { return m_map.begin(); }
iterator  FormalArgInfoAnnotationMap::end() { return m_map.end(); }
const_iterator  FormalArgInfoAnnotationMap::end() const { return m_map.end(); }

// FIXME: implement this
void FormalArgInfoAnnotationMap::compute() {
}

FormalArgInfoAnnotationMap * FormalArgInfoAnnotationMap::m_instance = 0;
PropertyHndlT FormalArgInfoAnnotationMap::m_handle = "FormalArgInfo";

}
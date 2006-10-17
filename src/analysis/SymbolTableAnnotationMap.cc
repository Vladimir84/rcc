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

// File: SymbolTableAnnotationMap.h
//
// Maps each procedure to a symbol table annotation.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/RccError.h>

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/DefVar.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/SymbolTable.h>
#include <analysis/VarBinding.h>
#include <analysis/VarInfo.h>

#include "SymbolTableAnnotationMap.h"

using namespace OA;

namespace RAnnot {
  
// ----- type definitions for readability -----

typedef SymbolTableAnnotationMap::MyKeyT MyKeyT;
typedef SymbolTableAnnotationMap::MyMappedT MyMappedT;
typedef SymbolTableAnnotationMap::iterator iterator;
typedef SymbolTableAnnotationMap::const_iterator const_iterator;

typedef CFG::Interface CFG;

//  ----- constructor/destructor ----- 
  
SymbolTableAnnotationMap::SymbolTableAnnotationMap(bool ownsAnnotations /* = true */)
  : m_computed(false),
    m_map()
  {}
  
SymbolTableAnnotationMap::~SymbolTableAnnotationMap() {}

// ----- singleton pattern -----

SymbolTableAnnotationMap * SymbolTableAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT SymbolTableAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void SymbolTableAnnotationMap::create() {
  m_instance = new SymbolTableAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

SymbolTableAnnotationMap * SymbolTableAnnotationMap::m_instance = 0;
PropertyHndlT SymbolTableAnnotationMap::m_handle = "SymbolTable";

//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & SymbolTableAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }

  return m_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT SymbolTableAnnotationMap::get(const MyKeyT & k) {
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
  
bool SymbolTableAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator SymbolTableAnnotationMap::begin() { return m_map.begin(); }
iterator SymbolTableAnnotationMap::end() { return m_map.end(); }
const_iterator SymbolTableAnnotationMap::begin() const { return m_map.begin(); }
const_iterator SymbolTableAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

/// Create the symbol table of each procedure.
void SymbolTableAnnotationMap::compute() {
  FuncInfoIterator fii(R_Analyst::get_instance()->get_scope_tree_root());

  // create empty symbol table for each scope
  for( ; fii.IsValid(); ++fii) {
    SymbolTable * st = new SymbolTable();
    m_map[HandleInterface::make_proc_h(fii.Current()->get_defn())] = st;
  }

  // for each def in each scope, add the appropriate info to the
  // scopes where the variable might live.
  for(fii.Reset(); fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();

    FuncInfo::mention_iterator mi;
    for(mi = fi->begin_mentions(); mi != fi->end_mentions(); ++mi) {
      // count only defs, not uses
      DefVar * def = dynamic_cast<DefVar *>(*mi);
      if (def == 0) continue;
      SEXP name = CAR(def->getMention_c());
      VarBinding * vb = getProperty(VarBinding, def->getMention_c());
      // for each scope in which the variable might be defined
      VarBinding::const_iterator vbi;
      for(vbi = vb->begin(); vbi != vb->end(); ++vbi) {
	if (const FundefLexicalScope * scope = dynamic_cast<const FundefLexicalScope *>(*vbi)) {
	  AnnotationBase * target_ab = m_map[HandleInterface::make_proc_h(scope->get_fundef())];
	  SymbolTable * target_st = dynamic_cast<SymbolTable *>(target_ab);
	  assert(target_st != 0);
	  SymbolTable::iterator it = target_st->find(name);
	  VarInfo * vi;
	  if (it == target_st->end()) {
	    vi = new VarInfo();
	    (*target_st)[name] = vi;
	  } else {
	    vi = it->second;
	  }
	  vi->insertDef(def);
	}
      }
    }
  }  
}

}
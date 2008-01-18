// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: OACallGraphAnnotation.h
//
// Annotation for call graph information. Contains a
// ProcHandleIterator that iterates through each procedure that may be
// called.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef OA_CALL_GRAPH_ANNOTATION_H
#define OA_CALL_GRAPH_ANNOTATION_H

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

namespace RAnnot {

class OACallGraphAnnotation : public AnnotationBase {
public:
  explicit OACallGraphAnnotation(const OA::OA_ptr<OA::ProcHandleIterator> iter);
  virtual ~OACallGraphAnnotation();

  OA::OA_ptr<OA::ProcHandleIterator> get_iterator() const;

  const OA::ProcHandle get_singleton_if_exists() const;

  // ----- methods that implement AnnotationBase -----
  AnnotationBase * clone();

  static PropertyHndlT handle();

  // ----- debugging -----
  std::ostream & dump(std::ostream & stream) const;

private:
  OA::OA_ptr<OA::ProcHandleIterator> m_iter;
};

} // end namespace RAnnot

#endif
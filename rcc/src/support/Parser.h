// -*- Mode: C++ -*-

#ifndef PARSER_H
#define PARSER_H

// Copyright (c) 2003-2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
//
// File: Parser.cc
//
// Interface to the R parser. Parses R code, returns S-expressions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <stdio.h>

#include <include/R/R_RInternals.h>

void init_R();
void parse_R(FILE *in_file, SEXP *exps[]);
SEXP parse_R_as_function(FILE *in_file);

#endif

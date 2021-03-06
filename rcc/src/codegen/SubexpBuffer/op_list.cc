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

// File: op_list.cc
//
// Output a list.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <assert.h>
#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <support/StringUtils.h>

#include <CodeGenUtils.h>
#include <ParseInfo.h>
#include <Dependence.h>
#include <Visibility.h>

using namespace std;

//*****************************************************************************
// forward declarations 
//*****************************************************************************

typedef enum {
  UNTAGGED,             // all LISTSXP with no tags
  UNTAGGED_LANG,        // first cons LANGSXP, the rest LISTSXP, no tags
  TAGGED,               // LISTSXPs with tags
  OTHER                 // any other list
} ListType;
static string getConsFunction(SEXP lst);
static bool list_is_tagged(SEXP e);
static int list_lang(SEXP e);

//*****************************************************************************
// interface operations 
//*****************************************************************************

Expression SubexpBuffer::op_list(SEXP list, string rho, bool literal,
				 Protection protection,
				 bool fullyEvaluatedResult /* = false */)
{
  if (protection == Unprotected) {
    assert(0 && "found unprotected op_list! I didn't think that could happen.\n");
  }
  int i;
  SEXP e;
  int unp_count = 0;
  Expression * cars;
  Expression * tags;
  DependenceType list_dep;
  if (list == R_NilValue) {
    return Expression::nil_exp;
  }

  int length = Rf_length(list);

  // output and store the car of each cons
  cars = new Expression[length];
  list_dep = CONST;
  e = list;
  for (i = 0; i < length; i++) {
    cars[i] = (literal ?
	       op_literal(CAR(e), rho) :
	       op_exp(e, rho, Protected, fullyEvaluatedResult));
    if (cars[i].dependence == DEPENDENT) {
      list_dep = DEPENDENT;
    }
    if (cars[i].dependence == DEPENDENT && cars[i].del_text != "") {
      unp_count++;
    }
    e = CDR(e);
  }

  string call;
  int lang = list_lang(list);  // number of LANGSXP conses at head
  if (list_is_tagged(list)) {
    // output and store tags
    tags = new Expression[length];
    e = list;
    for (i = 0; i < length; i++) {
      tags[i] = op_literal(TAG(e), rho);
      e = CDR(e);
    }
    call = "rcc_tagged_list(" + i_to_s(lang) + ", " + i_to_s(2 * length);
    // output in reverse order for consing
    for (i = length - 1; i >= 0; i--) {
      call += ", " + cars[i].var;
      call += ", " + tags[i].var;
    }
    delete [] tags;
  } else {
    call = "rcc_list(" + i_to_s(lang) + ", " + i_to_s(length);
    // output in reverse order for consing
    for (i = length - 1; i >= 0; i--) {
      call += ", " + cars[i].var;
    }
  }
  call += ")";
  delete [] cars;
  
  // if result is constant, assemble it in the constant pool
  SubexpBuffer * subexp = (list_dep == DEPENDENT ? 
			   this :
			   ParseInfo::global_constants);
  string var = subexp->new_var_unp();
  if (list_dep == DEPENDENT) {
    subexp->append_decls("SEXP " + var + ";\n");
  } else {
    subexp->append_decls("static SEXP " + var + ";\n");
  }

  if (unp_count > 0) {
    append_defs("UNPROTECT(" + i_to_s(unp_count) + ");\n");
  }
  string delete_text;
  subexp->append_defs(emit_assign(var, call));
  if (list_dep == DEPENDENT) {
    delete_text = unp(var);
  }
  return Expression(var, list_dep, VISIBLE, delete_text);
}

#if 0 
Expression SubexpBuffer::op_list(SEXP lst, string rho, bool literal, 
				 Protection protection,
				 bool fullyEvaluatedResult /* = false */ ) 
{
  if (lst == R_NilValue) { 

    //------------------------------------------------------------------
    // base case: empty list
    //------------------------------------------------------------------
    return Expression::nil_exp;

  } else {

    SEXP head = CAR(lst);
    SEXP rest = CDR(lst);

    //------------------------------------------------------------------
    // future optimization: if head is constant, we can avoid a protect 
    // for the rest of the list and head has no tag
    //
    // 14 September 2005 - John Mellor-Crummey
    //------------------------------------------------------------------
    Expression restExp = op_list(rest, rho, literal, 
				 Protected,
				 fullyEvaluatedResult);

    //------------------------------------------------------------------
    // must protect result of evaluating head if we need to produce a 
    // tag before consing the current node onto the list
    //------------------------------------------------------------------
    SEXP lstTag = TAG(lst);
    bool taggedResult = (lstTag != R_NilValue);

    Expression tag = Expression::nil_exp;

    if (taggedResult) {
      tag = ParseInfo::global_constants->op_literal(lstTag, rho);
    }

    Expression headExp = (literal ? 
		      op_literal(head, rho) :
		      op_exp(lst, rho, Unprotected, fullyEvaluatedResult));

    //------------------------------------------------------------------
    // result is non-constant if either head or rest are non-constant 
    //------------------------------------------------------------------
    bool resultNonConstant = (headExp.dependence == DEPENDENT ||
			      restExp.dependence == DEPENDENT);

    //------------------------------------------------------------------
    // if result is constant, assemble it in the constant pool
    //------------------------------------------------------------------
    SubexpBuffer *subexp = (resultNonConstant ? 
			    this :
			    ParseInfo::global_constants);

    Protection consProt = Unprotected;
    if (protection == Protected && !resultNonConstant) {
      consProt = Protected;
    }
    string out;
    int unprotcnt = 0;
    if (taggedResult) {
      out = subexp->appl3("tagged_cons", "", headExp.var, tag.var, restExp.var, 
			  consProt);
      if (!tag.del_text.empty()) unprotcnt++;
    } else {
      string consFn = getConsFunction(lst);
      out = subexp->appl2(consFn, "", headExp.var, restExp.var, consProt);
    }
    if (!headExp.del_text.empty()) unprotcnt++;
    if (!restExp.del_text.empty()) unprotcnt++;
    if (unprotcnt > 0)
      append_defs("UNPROTECT(" + i_to_s(unprotcnt) + ");\n");

    string deleteText;
    if (protection == Protected && resultNonConstant) {
      if (unprotcnt > 0)
	append_defs("SAFE_PROTECT(" + out + ");\n");
      else
	append_defs("PROTECT(" + out + ");\n");
      deleteText = unp(out);
    }
    
    return Expression(out, resultNonConstant ? DEPENDENT : CONST, VISIBLE, deleteText);
  } 
}
#endif

#if 0
{
  int i, len;
  len = Rf_length(e);
  string my_cons;
  if (e == R_NilValue) return Expression::nil_exp;
  if (len == 1) {
    switch (TYPEOF(e)) {
    case LISTSXP:
      my_cons = "cons";
      break;
    case LANGSXP:
      my_cons = "lcons";
      break;
    default:
      err("Internal error: bad call to op_list\n");
      return Expression::bogus_exp;  // never reached
    }
    Expression car = (literal ? op_literal(CAR(e), rho) :
		      op_exp(e, rho, Protected, 
			     fullyEvaluatedResult));
    string out;
    if (car.dependence == DEPENDENT) {
      if (TAG(e) == R_NilValue) {
	out = appl2(my_cons, car.var, "R_NilValue");
	del(car);
      } else {
	Expression tag = op_literal(TAG(e), rho);
	out = appl3("tagged_cons", car.var, tag.var, "R_NilValue");
	del(car);
	del(tag);
      }
      return Expression(out, DEPENDENT, VISIBLE, unp(out));
    } else {  // not dep
      if (TAG(e) == R_NilValue) {
	out = ParseInfo::global_constants->appl2(my_cons, car.var, "R_NilValue");
	ParseInfo::global_constants->del(car);
      } else {
	Expression tag = ParseInfo::global_constants->op_literal(TAG(e), rho);
	out = ParseInfo::global_constants->appl3("tagged_cons", car.var, tag.var, "R_NilValue");
	ParseInfo::global_constants->del(car);
	ParseInfo::global_constants->del(tag);
      }
      return Expression(out, CONST, VISIBLE, "");
    }
  } else {  // length >= 2
    string unp_cars = "";
    bool list_dep = FALSE;
    Expression *exps = new Expression[len];
    Expression *tags = new Expression[len];
    bool *langs = new bool[len];
    SEXP tmp_e = e;
    for(i=0; i<len; i++) {
      switch (TYPEOF(tmp_e)) {
      case LISTSXP:
	langs[i] = FALSE;
	break;
      case LANGSXP:
	langs[i] = TRUE;
	break;
      default:
	err("Internal error: bad call to op_list\n");
	return Expression::bogus_exp;   never reached
      }
      exps[i] = (literal? op_literal(CAR(tmp_e), rho) : 
		 op_exp(tmp_e, rho, Protected, fullyEvaluatedResult));
      tags[i] = (TAG(tmp_e) == R_NilValue ? Expression("", CONST, INVISIBLE, "") : op_literal(TAG(tmp_e), rho));
      if (exps[i].dependence == DEPENDENT) list_dep = TRUE;
      tmp_e = CDR(tmp_e);
    }
    SubexpBuffer tmp_buf = new_sb("tmp_list");
    string cdr = "R_NilValue";
    for(i=len-1; i>=0; i--) {
      my_cons = (langs[i] ? "lcons" : "cons");
      if (tags[i].var.empty()) {
	cdr = tmp_buf.appl2(my_cons, exps[i].var, cdr, Unprotected);
      } else {
	cdr = tmp_buf.appl3("tagged_cons", exps[i].var, tags[i].var, cdr, 
			    Unprotected);
      }
      unp_cars += exps[i].del_text;
    }
    delete [] exps;
    delete [] tags;
    delete [] langs;
    if (list_dep) {
      string handle = new_sexp();
      string defs;
      defs += tmp_buf.output();
      defs += "PROTECT(" + handle + " = " + cdr + ");\n";
      defs += unp_cars;
      append_defs(emit_in_braces(defs));
      return Expression(handle, list_dep ? DEPENDENT : CONST, VISIBLE, unp(handle));
    } else {
      string handle = ParseInfo::global_constants->new_sexp();
      string defs;
      defs += tmp_buf.output();
      defs += "PROTECT(" + handle + " = " + cdr + ");\n";
      defs += unp_cars;
      ParseInfo::global_constants->append_defs(emit_in_braces(defs));
      return Expression(handle, list_dep ? DEPENDENT : CONST, VISIBLE, "");
    }
  }
}
#endif



//*****************************************************************************
// private operations
//*****************************************************************************

static string getConsFunction(SEXP lst) 
{
  string consFn;
  switch (TYPEOF(lst)) {
  case LISTSXP:
    consFn = "cons";
    break;
  case LANGSXP:
    consFn = "lcons";
    break;
  default:
    // internal error
    assert(0);
  }
  return consFn;
}

static bool list_is_tagged(SEXP e) {
  assert(is_cons(e));
  while (e != R_NilValue) {
    if (TAG(e) != R_NilValue) {
      return true;
    }
    e = CDR(e);
  }
  return false;
}

static int list_lang(SEXP e) {
  assert(is_cons(e));
  int lang = 0;
  bool last_was_list = false;
  while (e != R_NilValue) {
    if (TYPEOF(e) == LANGSXP) {
      lang++;
      if (last_was_list) {
	assert(0);  // LANGSXP following a LISTSXP: I don't think this can happen.
      }
    }
    if (TYPEOF(e) == LISTSXP) {
      last_was_list = true;
    }
    e = CDR(e);
  }
  return lang;
}

// -*- Mode: C++ -*-
#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <string>

#include <include/Protection.h>
#include <include/R/R_RInternals.h>

#include <Output.h>
#include <CScope.h>

class CodeGen {
public:
  static Output op_exp(SEXP e, std::string rho,
		       bool fullyEvaluatedResult = false);
  static Output op_var_use(SEXP cell, std::string rho, 
			   Protection resultProtected, 
			   bool fullyEvaluatedResult);
  static Output op_closure(SEXP e, std::string rho);
  static Output op_primsxp(SEXP e, std::string rho);
  static Output op_lang(SEXP e, std::string rho);
  static Output op_promise(SEXP e);
  static Output op_begin(SEXP exp, std::string rho);
  static Output op_if(SEXP e, std::string rho);
  static Output op_for(SEXP e, std::string rho);
  static Output op_while(SEXP e, std::string rho);
  static Output op_return(SEXP e, std::string rho);
  static Output op_break(SEXP e, std::string rho);
  static Output op_fundef(SEXP e, std::string rho, std::string opt_R_name = "");
  static Output op_special(SEXP e, SEXP op, std::string rho);
  static Output op_builtin(SEXP e, SEXP op, std::string rho);
  static Output op_set(SEXP e, SEXP op, std::string rho);
  static Output op_subscriptset(SEXP e, std::string rho);
  static Output op_clos_app(std::string clos_h, SEXP args, std::string rho);
  static Output op_literal(SEXP e, std::string rho);
  static Output op_list(SEXP e,
			std::string rho,
			bool literal,
			bool promFuncArgList = false);
  static Output op_string(SEXP e);
  static Output op_vector(SEXP vec);

private:
  static const CScope m_scope;
};

#endif

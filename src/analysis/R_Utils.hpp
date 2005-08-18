#ifndef R_Utils_h
#define R_Utils_h

#include <assert.h>

#include <rinternals.h>

bool is_local_assign(const SEXP e);
bool is_free_assign(const SEXP e);
bool is_assign(const SEXP e);
bool is_simple_assign(const SEXP e);
SEXP assign_lhs_c(const SEXP e);
SEXP assign_rhs_c(const SEXP e);

bool is_fundef(const SEXP e);
SEXP fundef_args_c(const SEXP e);
SEXP fundef_body_c(const SEXP e);

bool is_struct_field(const SEXP e);
bool is_subscript(const SEXP e);
SEXP subscript_lhs_c(const SEXP e);
SEXP subscript_rhs_c(const SEXP e);
bool is_const(const SEXP e);
bool is_var(const SEXP e);

bool is_if(const SEXP e);
SEXP if_cond_c(const SEXP e);
SEXP if_truebody_c(const SEXP e);
SEXP if_falsebody_c(const SEXP e);
bool is_return(const SEXP e);
bool is_break(const SEXP e);
bool is_stop(const SEXP e);
bool is_next(const SEXP e);

bool is_loop(const SEXP e);
SEXP loop_body_c(const SEXP e);
bool is_for(const SEXP e);
SEXP for_iv_c(const SEXP e);
SEXP for_range_c(const SEXP e);
SEXP for_body_c(const SEXP e);
bool is_while(const SEXP e);
SEXP while_cond_c(const SEXP e);
SEXP while_body_c(const SEXP e);
bool is_repeat(const SEXP e);
SEXP repeat_body_c(const SEXP e);

bool is_paren_exp(const SEXP e);
SEXP paren_body_c(const SEXP e);
bool is_curly_list(const SEXP e);
SEXP curly_body(const SEXP e);

#endif

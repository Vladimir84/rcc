SEXP R_unary(SEXP, SEXP, SEXP);
SEXP rcc_R_binary(SEXP call, SEXP op, SEXP x, SEXP y);
SEXP rcc_do_arith(SEXP call, SEXP op, SEXP args, SEXP env);
SEXP VectorAssign(SEXP call, SEXP x, SEXP s, SEXP y);
int SubassignTypeFix(SEXP *x, SEXP *y, int stretch, int level, SEXP call);
SEXP EnlargeVector(SEXP x, R_len_t newlen);
SEXP DeleteListElements(SEXP x, SEXP which);
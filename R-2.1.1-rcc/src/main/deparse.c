/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2005  Robert Gentleman, Ross Ihaka and the
 *			      R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  IMPLEMENTATION NOTES:
 *
 *  Deparsing, has 3 layers.  The user interface, do_deparse, should
 *  not be called from an internal function, the actual deparsing needs
 *  to be done twice, once to count things up and a second time to put
 *  them into the string vector for return.  Printing this to a file
 *  is handled by the calling routine.
 *
 *
 *  INDENTATION:
 *
 *  Indentation is carried out in the routine printtab2buff at the
 *  botton of this file.  It seems like this should be settable via
 *  options.
 *
 *
 *  GLOBAL VARIABLES:
 *
 *  linenumber:	 counts the number of lines that have been written,
 *		 this is used to setup storage for deparsing.
 *
 *  len:	 counts the length of the current line, it will be
 *		 used to determine when to break lines.
 *
 *  incurly:	 keeps track of whether we are inside a curly or not,
 *		 this affects the printing of if-then-else.
 *
 *  inlist:	 keeps track of whether we are inside a list or not,
 *		 this affects the printing of if-then-else.
 *
 *  startline:	 indicator TRUE=start of a line (so we can tab out to
 *		 the correct place).
 *
 *  indent:	 how many tabs should be written at the start of
 *		 a line.
 *
 *  buff:	 contains the current string, we attempt to break
 *		 lines at cutoff, but can unlimited length.
 *
 *  lbreak:	 often used to indicate whether a line has been
 *		 broken, this makes sure that that indenting behaves
 *		 itself.
 */

/*
* The code here used to use static variables to share values
* across the different routines. These have now been collected
* into a struct named  LocalParseData and this is explicitly
* passed between the different routines. This avoids the needs
* for the global variables and allows multiple evaluators, potentially
* in different threads, to work on their own independent copies
* that are local to their call stacks. This avoids any issues
* with interrupts, etc. not restoring values.

* The previous issue with the global "cutoff" variable is now implemented
* by creating a deparse1WithCutoff() routine which takes the cutoff from
* the caller and passes this to the different routines as a member of the
* LocalParseData struct. Access to the deparse1() routine remains unaltered.
* This is exactly as Ross had suggested ...
*
* One possible fix is to restructure the code with another function which
* takes a cutoff value as a parameter.	 Then "do_deparse" and "deparse1"
* could each call this deeper function with the appropriate argument.
* I wonder why I didn't just do this? -- it would have been quicker than
* writing this note.  I guess it needs a bit more thought ...
*/

/* <UTF8> char here is either ASCII or handled as a whole.
   E.g. backquotify should work.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Print.h>
#include <Fileio.h>

#define BUFSIZE 512

#define MIN_Cutoff 20
#define DEFAULT_Cutoff 60
#define MAX_Cutoff (BUFSIZE - 12)
/* ----- MAX_Cutoff  <	BUFSIZE !! */

extern int isValidName(char*);

#include "RBufferUtils.h"

typedef R_StringBuffer DeparseBuffer;

typedef struct {
    int linenumber;
    int len;
    int incurly;
    int inlist;
    Rboolean startline; /* = TRUE; */
    int indent;
    SEXP strvec;

    DeparseBuffer buffer;

    int cutoff;
    int backtick;
    int opts;
    int sourceable;
} LocalParseData;

static SEXP deparse1WithCutoff(SEXP call, Rboolean abbrev, int cutoff,
			       Rboolean backtick, int opts);
static void args2buff(SEXP, int, int, LocalParseData *);
static void deparse2buff(SEXP, LocalParseData *);
static void print2buff(char *, LocalParseData *);
static void printtab2buff(int, LocalParseData *);
static void scalar2buff(SEXP, LocalParseData *);
static void writeline(LocalParseData *);
static void vector2buff(SEXP, LocalParseData *);
static void vec2buff(SEXP, LocalParseData *);
static void linebreak(Rboolean *lbreak, LocalParseData *);
static void deparse2(SEXP, SEXP, LocalParseData *);

void R_AllocStringBuffer(int blen, DeparseBuffer *buf)
{
    if(blen >= 0) {
	if(blen*sizeof(char) < buf->bufsize) return;
	blen = (blen+1)*sizeof(char);
	if(blen < buf->defaultSize) blen = buf->defaultSize;
	if(buf->data == NULL){
		buf->data = (char *) malloc(blen);
		buf->data[0] = '\0';
	} else
		buf->data = (char *) realloc(buf->data, blen);
	buf->bufsize = blen;
	if(!buf->data) {
		buf->bufsize = 0;
		error(_("could not allocate memory in C function 'R_AllocStringBuffer'"));
	}
    } else {
	if(buf->bufsize == buf->defaultSize) return;
	free(buf->data);
	buf->data = (char *) malloc(buf->defaultSize);
	buf->bufsize = buf->defaultSize;
    }
}

void R_FreeStringBuffer(DeparseBuffer *buf)
{
    if (buf->data != NULL) {
	free(buf->data);
	buf->bufsize = 0;
	buf->data = NULL;
    }
}

SEXP do_deparse(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP ca1;
    int  cut0, backtick, opts;

    checkArity(op, args);

    if(length(args) < 1) errorcall(call, _("too few arguments"));

    ca1 = CAR(args); args = CDR(args);
    cut0 = DEFAULT_Cutoff;
    if(!isNull(CAR(args))) {
	cut0 = asInteger(CAR(args));
	if(cut0 == NA_INTEGER|| cut0 < MIN_Cutoff || cut0 > MAX_Cutoff) {
	    warning(_("invalid 'cutoff' for deparse, using default"));
	    cut0 = DEFAULT_Cutoff;
	}
    }
    args = CDR(args);
    backtick = 0;
    if(!isNull(CAR(args)))
	backtick = asLogical(CAR(args));
    args = CDR(args);
    opts = SHOWATTRIBUTES;
    if(!isNull(CAR(args)))
    	opts = asInteger(CAR(args));
    ca1 = deparse1WithCutoff(ca1, 0, cut0, backtick, opts);
    return ca1;
}

SEXP deparse1(SEXP call, Rboolean abbrev, int opts)
{
    Rboolean backtick = TRUE;
    return(deparse1WithCutoff(call, abbrev, DEFAULT_Cutoff, backtick,
    			      opts));
}

static SEXP deparse1WithCutoff(SEXP call, Rboolean abbrev, int cutoff,
			       Rboolean backtick, int opts)
{
/* Arg. abbrev:
	If abbrev is TRUE, then the returned value
	is a STRSXP of length 1 with at most 10 characters.
	This is used for plot labelling etc.
*/
    SEXP svec;
    int savedigits;
    /* The following gives warning (when "-pedantic" is used):
       In function `deparse1WithCutoff': initializer element
       is not computable at load time -- and its because of R_NilValue
       (RH 7.1 gcc 2.96  wrongly gives an error with "-pedantic")
    */
    LocalParseData localData =
	    {0, 0, 0, 0, /*startline = */TRUE, 0,
	     NULL,
	     /*DeparseBuffer=*/{NULL, 0, BUFSIZE},
	     DEFAULT_Cutoff, FALSE, 0, TRUE};
    DeparseBuffer *buffer = &localData.buffer;
    localData.cutoff = cutoff;
    localData.backtick = backtick;
    localData.opts = opts;
    localData.strvec = R_NilValue;

    PrintDefaults(R_NilValue);/* from global options() */
    savedigits = R_print.digits;
    R_print.digits = DBL_DIG;/* MAX precision */

    svec = R_NilValue;
    deparse2(call, svec, &localData);/* just to determine linenumber..*/
    PROTECT(svec = allocVector(STRSXP, localData.linenumber));
    deparse2(call, svec, &localData);
    UNPROTECT(1);
    if (abbrev) {
	R_AllocStringBuffer(0, buffer);
	buffer->data[0] = '\0';
	strncat(buffer->data, CHAR(STRING_ELT(svec, 0)), 10);
	if (strlen(CHAR(STRING_ELT(svec, 0))) > 10)
	    strcat(buffer->data, "...");
	svec = mkString(buffer->data);
    }
    R_print.digits = savedigits;
    R_FreeStringBuffer(buffer);
    if ((opts & WARNINCOMPLETE) && !localData.sourceable)
    	warning(_("deparse may be incomplete"));
    return svec;
}

/* deparse1line uses the maximum cutoff rather than the default */
/* This is needed in terms.formula, where we must be able */
/* to deparse a term label into a single line of text so */
/* that it can be reparsed correctly */
SEXP deparse1line(SEXP call, Rboolean abbrev)
{
   SEXP temp;
   Rboolean backtick=TRUE;

   temp = deparse1WithCutoff(call, abbrev, MAX_Cutoff, backtick,
   			     SIMPLEDEPARSE);
   return(temp);
}

#include "Rconnections.h"

SEXP do_dput(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP saveenv, tval;
    int i, ifile, res;
    Rboolean wasopen, havewarned = FALSE, opts;
    Rconnection con = (Rconnection) 1; /* stdout */

    checkArity(op, args);

    tval = CAR(args);
    saveenv = R_NilValue;	/* -Wall */
    if (TYPEOF(tval) == CLOSXP) {
	PROTECT(saveenv = CLOENV(tval));
	SET_CLOENV(tval, R_GlobalEnv);
    }
    else if (TYPEOF(tval) == RCC_CLOSXP) {
	PROTECT(saveenv = RCC_CLOSXP_CLOENV(tval));
	RCC_CLOSXP_SET_CLOENV(tval, R_GlobalEnv);
    }
    opts = SHOWATTRIBUTES;
    if(!isNull(CADDR(args)))
       	opts = asInteger(CADDR(args));

    tval = deparse1(tval, 0, opts);
    if (TYPEOF(CAR(args)) == CLOSXP) {
	SET_CLOENV(CAR(args), saveenv);
	UNPROTECT(1);
    }
    else if (TYPEOF(tval) == RCC_CLOSXP) {
	RCC_CLOSXP_SET_CLOENV(CAR(args), saveenv);
	UNPROTECT(1);
    }
    ifile = asInteger(CADR(args));

    wasopen = 1;
    if (ifile != 1) {
	con = getConnection(ifile);
	wasopen = con->isopen;
	if(!wasopen)
	    if(!con->open(con)) error(_("cannot open the connection"));
    }/* else: "Stdout" */
    for (i = 0; i < LENGTH(tval); i++)
	if (ifile == 1)
	    Rprintf("%s\n", CHAR(STRING_ELT(tval, i)));
	else {
	    res = Rconn_printf(con, "%s\n", CHAR(STRING_ELT(tval, i)));
	    if(!havewarned &&
	       res < strlen(CHAR(STRING_ELT(tval, i))) + 1)
		warningcall(call, _("wrote too few characters"));
	}
    if (!wasopen) con->close(con);
    return (CAR(args));
}

SEXP do_dump(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP file, names, o, objs, tval, source;
    int i, j, nobjs, res;
    Rboolean wasopen, havewarned = FALSE, evaluate;
    Rconnection con;
    int opts;
    char *obj_name;

    checkArity(op, args);

    names = CAR(args);
    file = CADR(args);
    if(!isString(names))
	errorcall(call, _("character arguments expected"));
    nobjs = length(names);
    if(nobjs < 1 || length(file) < 1)
	errorcall(call, _("zero length argument"));
    source = CADDR(args);
    if (source != R_NilValue && TYPEOF(source) != ENVSXP)
	error(_("bad environment"));
    opts = FORSOURCING;
    if (!isNull(CADDDR(args)))
    	opts = asInteger(CADDDR(args));
    evaluate = asLogical(CAD4R(args));
    if (!evaluate) opts |= DELAYPROMISES;

    PROTECT(o = objs = allocList(nobjs));

    for (j = 0; j < nobjs; j++, o = CDR(o)) {
	SET_TAG(o, install(CHAR(STRING_ELT(names, j))));
	SETCAR(o, findVar(TAG(o), source));
	if (CAR(o) == R_UnboundValue)
	    error(_("Object \"%s\" not found"), CHAR(PRINTNAME(TAG(o))));
    }
    o = objs;
    if(INTEGER(file)[0] == 1) {
	for (i = 0; i < nobjs; i++) {
	    obj_name = CHAR(STRING_ELT(names, i));
	    /* figure out if we need to quote the name */
	    if(!isValidName(obj_name))
		Rprintf("\"%s\" <-\n", obj_name);
	    else
		Rprintf("%s <-\n", obj_name);
	    tval = deparse1(CAR(o), 0, opts);
	    for (j = 0; j < LENGTH(tval); j++) {
		Rprintf("%s\n", CHAR(STRING_ELT(tval, j)));
	    }
	    o = CDR(o);
	}
    }
    else {
	con = getConnection(INTEGER(file)[0]);
	wasopen = con->isopen;
	if (!wasopen)
	    if(!con->open(con)) error(_("cannot open the connection"));
	for (i = 0; i < nobjs; i++) {
	    res = Rconn_printf(con, "\"%s\" <-\n", CHAR(STRING_ELT(names, i)));
	    if(!havewarned &&
	       res < strlen(CHAR(STRING_ELT(names, i))) + 4)
		warningcall(call, _("wrote too few characters"));
	    tval = deparse1(CAR(o), 0, opts);
	    for (j = 0; j < LENGTH(tval); j++) {
		res = Rconn_printf(con, "%s\n", CHAR(STRING_ELT(tval, j)));
		if(!havewarned &&
		   res < strlen(CHAR(STRING_ELT(tval, j))) + 1)
		    warningcall(call, _("wrote too few characters"));
	    }
	    o = CDR(o);
	}
	if (!wasopen) con->close(con);
    }

    UNPROTECT(1);
    R_Visible = 0;
    return names;
}

static void linebreak(Rboolean *lbreak, LocalParseData *d)
{
    if (d->len > d->cutoff) {
	if (!*lbreak) {
	    *lbreak = TRUE;
	    d->indent++;
	}
	writeline(d);
    }
}

static void deparse2(SEXP what, SEXP svec, LocalParseData *d)
{
    d->strvec = svec;
    d->linenumber = 0;
    d->indent = 0;
    deparse2buff(what, d);
    writeline(d);
}


/* curlyahead looks at s to see if it is a list with
   the first op being a curly.  You need this kind of
   lookahead info to print if statements correctly.  */
static Rboolean
curlyahead(SEXP s)
{
    if (isList(s) || isLanguage(s))
	if (TYPEOF(CAR(s)) == SYMSXP && CAR(s) == install("{"))
	    return TRUE;
    return FALSE;
}

/* needsparens looks at an arg to a unary or binary operator to
   determine if it needs to be parenthesized when deparsed
   mainop is a unary or binary operator,
   arg is an argument to it, on the left if left == 1 */

static Rboolean needsparens(PPinfo mainop, SEXP arg, unsigned int left)
{
    PPinfo arginfo;
    if (TYPEOF(arg) == LANGSXP) {
	if (TYPEOF(CAR(arg)) == SYMSXP) {
	    if ((TYPEOF(SYMVALUE(CAR(arg))) == BUILTINSXP) ||
		(TYPEOF(SYMVALUE(CAR(arg))) == SPECIALSXP)) {
		arginfo = PPINFO(SYMVALUE(CAR(arg)));
		switch(arginfo.kind) {
		case PP_BINARY:	      /* Not all binary ops are binary! */
		case PP_BINARY2:
		    switch(length(CDR(arg))) {
		    case 1:
		    	if (!left)
		    	    return FALSE;
			if (arginfo.precedence == PREC_SUM)   /* binary +/- precedence upgraded as unary */
			    arginfo.precedence = PREC_SIGN;
		    case 2:
			break;
		    default:
			return FALSE;
		    }
		case PP_ASSIGN:
		case PP_ASSIGN2:
		case PP_SUBSET:
		case PP_UNARY:
		case PP_DOLLAR:
		    if (mainop.precedence > arginfo.precedence
			|| (mainop.precedence == arginfo.precedence && left == mainop.rightassoc)) {
			return TRUE;
		    }
		    break;
		case PP_FOR:
		case PP_IF:
		case PP_WHILE:
		case PP_REPEAT:
		    return left == 1;
		    break;
		default:
		    return FALSE;
		}
	    }
	}
    }
    else if ((TYPEOF(arg) == CPLXSXP) && (length(arg) == 1)) {
	if (mainop.precedence > PREC_SUM
	    || (mainop.precedence == PREC_SUM && left == mainop.rightassoc)) {
	    return TRUE;
	}
    }
    return FALSE;
}

/* check for attributes other than function source */
static Rboolean hasAttributes(SEXP s)
{
    SEXP a = ATTRIB(s);
    if (length(a) > 1
    	|| (length(a) == 1
    		&& ((TYPEOF(s) != CLOSXP && TYPEOF(s) != RCC_CLOSXP) || 
		    TAG(a) != R_SourceSymbol)))
    	return(TRUE);
    else
    	return(FALSE);
}

static void attr1(SEXP s, LocalParseData *d)
{
    if(hasAttributes(s))
	print2buff("structure(", d);
}

static void attr2(SEXP s, LocalParseData *d)
{
    Rboolean localOpts = d->opts;

    if(hasAttributes(s)) {
	SEXP a = ATTRIB(s);
	while(!isNull(a)) {
	    if(TAG(a) != R_SourceSymbol) {
		print2buff(", ", d);
		if(TAG(a) == R_DimSymbol) {
		    print2buff(".Dim", d);
		}
		else if(TAG(a) == R_DimNamesSymbol) {
		    print2buff(".Dimnames", d);
		}
		else if(TAG(a) == R_NamesSymbol) {
		    print2buff(".Names", d);
		}
		else if(TAG(a) == R_TspSymbol) {
		    print2buff(".Tsp", d);
		}
		else if(TAG(a) == R_LevelsSymbol) {
		    print2buff(".Label", d);
		}
		else {
		    /* TAG(a) might contain spaces etc */
		    char *tag = CHAR(PRINTNAME(TAG(a)));
		    d->opts = SIMPLEDEPARSE;
		    if(isValidName(tag))
			deparse2buff(TAG(a), d);
		    else {
			print2buff("\"", d);
			deparse2buff(TAG(a), d);
			print2buff("\"", d);
		    }
		    d->opts = localOpts;
		}
		print2buff(" = ", d);
		deparse2buff(CAR(a), d);
	    }
	    a = CDR(a);
	}
	print2buff(")", d);
    }
}


static void printcomment(SEXP s, LocalParseData *d)
{
    SEXP cmt;
    int i, ncmt;

    /* look for old-style comments first */

    if(isList(TAG(s)) && !isNull(TAG(s))) {
	for (s = TAG(s); s != R_NilValue; s = CDR(s)) {
	    print2buff(CHAR(STRING_ELT(CAR(s), 0)), d);
	    writeline(d);
	}
    }
    else {
	cmt = getAttrib(s, R_CommentSymbol);
	ncmt = length(cmt);
	for(i = 0 ; i < ncmt ; i++) {
	    print2buff(CHAR(STRING_ELT(cmt, i)), d);
	    writeline(d);
	}
    }
}

static char * backquotify(char *s)
{
    static char buf[120];
    char *t = buf;

    /* If a symbol is not a valid name, put it in backquotes, escaping
     * any backquotes in the string itself */

    /* NOTE: This could be fragile if sufficiently weird names are
     * used. Ideally, we should insert backslash escapes, etc. */

    if (isValidName(s) || *s == '\0') return s;

    *t++ = '`';
#ifdef SUPPORT_MBCS
    if(mbcslocale && !utf8locale) {
	mbstate_t mb_st; int j, used;
	mbs_init(&mb_st);
	while( (used = Mbrtowc(NULL, s, MB_CUR_MAX, &mb_st)) ) {
	    if ( *s  == '`' || *s == '\\' ) *t++ = '\\';
	    for(j = 0; j < used; j++) *t++ = *s++;
	}
    } else
#endif
	while ( *s ) {
	    if ( *s  == '`' || *s == '\\' ) *t++ = '\\';
	    *t++ = *s++;
	}
    *t++ = '`';
    *t = '\0';
    return buf;
}

/* This is the recursive part of deparsing. */


static void deparse2buff(SEXP s, LocalParseData *d)
{
    PPinfo fop;
    Rboolean lookahead = FALSE, lbreak = FALSE, parens;
    Rboolean localOpts = d->opts;
    SEXP op, t;
    char tpb[120];
    int i, n;

    switch (TYPEOF(s)) {
    case NILSXP:
	print2buff("NULL", d);
	break;
    case SYMSXP:
    	if (localOpts & QUOTEEXPRESSIONS) {
	    attr1(s, d);
	    print2buff("quote(", d);
	}
	if (d->backtick)
	    print2buff(backquotify(CHAR(PRINTNAME(s))), d);
	else
	    print2buff(CHAR(PRINTNAME(s)), d);
	if (localOpts & QUOTEEXPRESSIONS) {
	    print2buff(")", d);
	    attr2(s, d);
	}
	break;
    case CHARSXP:
	print2buff(CHAR(s), d);
	break;
    case SPECIALSXP:
    case BUILTINSXP:
	snprintf(tpb, 120, ".Primitive(\"%s\")", PRIMNAME(s));
	print2buff(tpb, d);
	break;
    case PROMSXP:
	if(d->opts & DELAYPROMISES) {
	    d->sourceable = FALSE;
	    print2buff("<promise: ", d);
	    d->opts &= ~QUOTEEXPRESSIONS; /* don't want delay(quote()) */
	    deparse2buff(PREXPR(s), d);
	    d->opts = localOpts;
	    print2buff(">", d);
	} else {
	    PROTECT(s = eval(s, NULL)); /* eval uses env of promise */
	    deparse2buff(s, d);
	    UNPROTECT(1);
	}
	break;
    case RCC_FUNSXP:
      {
	SEXP body = RCC_FUNSXP_BODY_EXPR(s);
	d->opts = SIMPLEDEPARSE;
	deparse2buff(body, d);
	d->opts = localOpts;
	break;
      }
    case CLOSXP:
    case RCC_CLOSXP:
        if (localOpts & SHOWATTRIBUTES) attr1(s, d);
        if ((d->opts & USESOURCE) && (n = length(t = getAttrib(s, R_SourceSymbol))) > 0) {
    	    for(i = 0 ; i < n ; i++) {
	    	print2buff(CHAR(STRING_ELT(t, i)), d);
	    	writeline(d);
	    }
	} else {
	    SEXP formals, body;
	    if (TYPEOF(s) == CLOSXP) {
	      formals = FORMALS(s);
	      body = BODY_EXPR(s);
	    } else {
	      formals = RCC_CLOSXP_FORMALS(s);
	      body = RCC_CLOSXP_FUN(s);
	    }
	 
	    d->opts = SIMPLEDEPARSE;
	    print2buff("function (", d);
	    args2buff(formals, 0, 1, d);
	    print2buff(") ", d);

	    writeline(d);
	    deparse2buff(body, d);
	    d->opts = localOpts;
	}
	if (localOpts & SHOWATTRIBUTES) attr2(s, d);
	break;

    case ENVSXP:
        d->sourceable = FALSE;
	print2buff("<environment>", d);
	break;
    case VECSXP:
	if (localOpts & SHOWATTRIBUTES) attr1(s, d);
	print2buff("list(", d);
	vec2buff(s, d);
	print2buff(")", d);
	if (localOpts & SHOWATTRIBUTES) attr2(s, d);
	break;
    case EXPRSXP:
    	if (localOpts & SHOWATTRIBUTES) attr1(s, d);
	if(length(s) <= 0)
	    print2buff("expression()", d);
	else {
	    print2buff("expression(", d);
	    d->opts = SIMPLEDEPARSE;
	    vec2buff(s, d);
	    d->opts = localOpts;
	    print2buff(")", d);
	}
	if (localOpts & SHOWATTRIBUTES) attr2(s, d);
	break;
    case LISTSXP:
	if (localOpts & SHOWATTRIBUTES) attr1(s, d);
	print2buff("list(", d);
	d->inlist++;
	for (t=s ; CDR(t) != R_NilValue ; t=CDR(t) ) {
	    if( TAG(t) != R_NilValue ) {
		d->opts = SIMPLEDEPARSE;
		deparse2buff(TAG(t), d);
		d->opts = localOpts;
		print2buff(" = ", d);
	    }
	    deparse2buff(CAR(t), d);
	    print2buff(", ", d);
	}
	if( TAG(t) != R_NilValue ) {
	    d->opts = SIMPLEDEPARSE;
	    deparse2buff(TAG(t), d);
	    d->opts = localOpts;
	    print2buff(" = ", d);
	}
	deparse2buff(CAR(t), d);
	print2buff(")", d);
	d->inlist--;
	if (localOpts & SHOWATTRIBUTES) attr2(s, d);
	break;
    case LANGSXP:
	printcomment(s, d);
	if (localOpts & QUOTEEXPRESSIONS) {
	    print2buff("quote(", d);
	    d->opts = SIMPLEDEPARSE;
	}
	if (TYPEOF(CAR(s)) == SYMSXP) {
	    if ((TYPEOF(SYMVALUE(CAR(s))) == BUILTINSXP) ||
		(TYPEOF(SYMVALUE(CAR(s))) == SPECIALSXP)) {
		op = CAR(s);
		fop = PPINFO(SYMVALUE(op));
		s = CDR(s);
		if (fop.kind == PP_BINARY) {
		    switch (length(s)) {
		    case 1:
			fop.kind = PP_UNARY;
			if (fop.precedence == PREC_SUM)   /* binary +/- precedence upgraded as unary */
			    fop.precedence = PREC_SIGN;
			break;
		    case 2:
			break;
		    default:
			fop.kind = PP_FUNCALL;
			break;
		    }
		}
		else if (fop.kind == PP_BINARY2) {
		    if (length(s) != 2)
			fop.kind = PP_FUNCALL;
		}
		switch (fop.kind) {
		case PP_IF:
		    print2buff("if (", d);
		    /* print the predicate */
		    deparse2buff(CAR(s), d);
		    print2buff(") ", d);
		    if (d->incurly && !d->inlist ) {
			lookahead = curlyahead(CAR(CDR(s)));
			if (!lookahead) {
			    writeline(d);
			    d->indent++;
			}
		    }
		    /* need to find out if there is an else */
		    if (length(s) > 2) {
			deparse2buff(CAR(CDR(s)), d);
			if (d->incurly && !d->inlist) {
			    writeline(d);
			    if (!lookahead)
				d->indent--;
			}
			else
			    print2buff(" ", d);
			print2buff("else ", d);
			deparse2buff(CAR(CDDR(s)), d);
		    }
		    else {
			deparse2buff(CAR(CDR(s)), d);
			if (d->incurly && !lookahead && !d->inlist )
			    d->indent--;
		    }
		    break;
		case PP_WHILE:
		    print2buff("while (", d);
		    deparse2buff(CAR(s), d);
		    print2buff(") ", d);
		    deparse2buff(CADR(s), d);
		    break;
		case PP_FOR:
		    print2buff("for (", d);
		    deparse2buff(CAR(s), d);
		    print2buff(" in ", d);
		    deparse2buff(CADR(s), d);
		    print2buff(") ", d);
		    deparse2buff(CADR(CDR(s)), d);
		    break;
		case PP_REPEAT:
		    print2buff("repeat ", d);
		    deparse2buff(CAR(s), d);
		    break;
		case PP_CURLY:
		    print2buff("{", d);
		    d->incurly += 1;
		    d->indent++;
		    writeline(d);
		    while (s != R_NilValue) {
			deparse2buff(CAR(s), d);
			writeline(d);
			s = CDR(s);
		    }
		    d->indent--;
		    print2buff("}", d);
		    d->incurly -= 1;
		    break;
		case PP_PAREN:
		    print2buff("(", d);
		    deparse2buff(CAR(s), d);
		    print2buff(")", d);
		    break;
		case PP_SUBSET:
		    deparse2buff(CAR(s), d);
		    if (PRIMVAL(SYMVALUE(op)) == 1)
			print2buff("[", d);
		    else
			print2buff("[[", d);
		    args2buff(CDR(s), 0, 0, d);
		    if (PRIMVAL(SYMVALUE(op)) == 1)
			print2buff("]", d);
		    else
			print2buff("]]", d);
		    break;
		case PP_FUNCALL:
		case PP_RETURN:
		    if (isValidName(CHAR(PRINTNAME(op))))
			print2buff(CHAR(PRINTNAME(op)), d);
		    else {
			print2buff("\"", d);
			print2buff(CHAR(PRINTNAME(op)), d);
			print2buff("\"", d);
		    }
		    print2buff("(", d);
		    d->inlist++;
		    args2buff(s, 0, 0, d);
		    d->inlist--;
		    print2buff(")", d);
		    break;
		case PP_FOREIGN:
		    print2buff(CHAR(PRINTNAME(op)), d);
		    print2buff("(", d);
		    d->inlist++;
		    args2buff(s, 1, 0, d);
		    d->inlist--;
		    print2buff(")", d);
		    break;
		case PP_FUNCTION:
		    printcomment(s, d);
		    if (!(d->opts & USESOURCE) || isNull(CADDR(s))) {
		    	print2buff(CHAR(PRINTNAME(op)), d);
		    	print2buff("(", d);
		    	args2buff(FORMALS(s), 0, 1, d);
		    	print2buff(") ", d);
		    	deparse2buff(CADR(s), d);
		    } else {
			s = CADDR(s);
			n = length(s);
	    	    	for(i = 0 ; i < n ; i++) {
	    		    print2buff(CHAR(STRING_ELT(s, i)), d);
	    		    writeline(d);
			}
		    }
		    break;
		case PP_ASSIGN:
		case PP_ASSIGN2:
		    if ((parens = needsparens(fop, CAR(s), 1)))
			print2buff("(", d);
		    deparse2buff(CAR(s), d);
		    if (parens)
			print2buff(")", d);
		    print2buff(" ", d);
		    print2buff(CHAR(PRINTNAME(op)), d);
		    print2buff(" ", d);
		    if ((parens = needsparens(fop, CADR(s), 0)))
			print2buff("(", d);
		    deparse2buff(CADR(s), d);
		    if (parens)
			print2buff(")", d);
		    break;
		case PP_DOLLAR:
		    if ((parens = needsparens(fop, CAR(s), 1)))
			print2buff("(", d);
		    deparse2buff(CAR(s), d);
		    if (parens)
			print2buff(")", d);
		    print2buff(CHAR(PRINTNAME(op)), d);
		    /*temp fix to handle printing of x$a's */
		    if( isString(CADR(s)) &&
			isValidName(CHAR(STRING_ELT(CADR(s), 0))))
			deparse2buff(STRING_ELT(CADR(s), 0), d);
		    else {
			if ((parens = needsparens(fop, CADR(s), 0)))
			    print2buff("(", d);
			deparse2buff(CADR(s), d);
			if (parens)
			    print2buff(")", d);
		    }
		    break;
		case PP_BINARY:
		    if ((parens = needsparens(fop, CAR(s), 1)))
			print2buff("(", d);
		    deparse2buff(CAR(s), d);
		    if (parens)
			print2buff(")", d);
		    print2buff(" ", d);
		    print2buff(CHAR(PRINTNAME(op)), d);
		    print2buff(" ", d);
		    linebreak(&lbreak, d);
		    if ((parens = needsparens(fop, CADR(s), 0)))
			print2buff("(", d);
		    deparse2buff(CADR(s), d);
		    if (parens)
			print2buff(")", d);
		    if (lbreak) {
			d->indent--;
			lbreak = FALSE;
		    }
		    break;
		case PP_BINARY2:	/* no space between op and args */
		    if ((parens = needsparens(fop, CAR(s), 1)))
			print2buff("(", d);
		    deparse2buff(CAR(s), d);
		    if (parens)
			print2buff(")", d);
		    print2buff(CHAR(PRINTNAME(op)), d);
		    if ((parens = needsparens(fop, CADR(s), 0)))
			print2buff("(", d);
		    deparse2buff(CADR(s), d);
		    if (parens)
			print2buff(")", d);
		    break;
		case PP_UNARY:
		    print2buff(CHAR(PRINTNAME(op)), d);
		    if ((parens = needsparens(fop, CAR(s), 0)))
			print2buff("(", d);
		    deparse2buff(CAR(s), d);
		    if (parens)
			print2buff(")", d);
		    break;
		case PP_BREAK:
		    print2buff("break", d);
		    break;
		case PP_NEXT:
		    print2buff("next", d);
		    break;
		case PP_SUBASS:
		    print2buff("\"", d);
		    print2buff(CHAR(PRINTNAME(op)), d);
		    print2buff("\"(", d);
		    args2buff(s, 0, 0, d);
		    print2buff(")", d);
		    break;
		default:
		    d->sourceable = FALSE;
		    UNIMPLEMENTED("deparse2buff");
		}
	    }
	    else {
		if(isSymbol(CAR(s)) && isUserBinop(CAR(s))) {
		    op = CAR(s);
		    s = CDR(s);
		    deparse2buff(CAR(s), d);
		    print2buff(" ", d);
		    print2buff(CHAR(PRINTNAME(op)), d);
		    print2buff(" ", d);
		    linebreak(&lbreak, d);
		    deparse2buff(CADR(s), d);
		    if (lbreak) {
			d->indent--;
			lbreak = FALSE;
		    }
		    break;
		}
		else {
		    SEXP val = R_NilValue; /* -Wall */
		    if (isSymbol(CAR(s))) {
			val = SYMVALUE(CAR(s));
			if (TYPEOF(val) == PROMSXP)
			    val = eval(val, R_NilValue);
		    }
		    if ( isSymbol(CAR(s))
		      && TYPEOF(val) == CLOSXP
		      && streql(CHAR(PRINTNAME(CAR(s))), "::") ){ /*  :: is special case */
		    	deparse2buff(CADR(s), d);
		    	print2buff("::", d);
			deparse2buff(CADDR(s), d);
		    }
		    else if ( isSymbol(CAR(s))
		      && TYPEOF(val) == CLOSXP
		      && streql(CHAR(PRINTNAME(CAR(s))), ":::") ){ /*  ::: is special case */
		    	deparse2buff(CADR(s), d);
		    	print2buff(":::", d);
			deparse2buff(CADDR(s), d);
		    }
		    else {
			if ( isSymbol(CAR(s)) ){
			    if ( !isValidName(CHAR(PRINTNAME(CAR(s)))) ){

				print2buff("\"", d);
				print2buff(CHAR(PRINTNAME(CAR(s))), d);
				print2buff("\"", d);
			    } else
				print2buff(CHAR(PRINTNAME(CAR(s))), d);
			}
			else
			    deparse2buff(CAR(s), d);
			print2buff("(", d);
			args2buff(CDR(s), 0, 0, d);
			print2buff(")", d);
		    }
		}
	    }
	}
	else if (TYPEOF(CAR(s)) == CLOSXP || TYPEOF(CAR(s)) == SPECIALSXP ||
		 TYPEOF(CAR(s)) == RCC_CLOSXP || TYPEOF(CAR(s)) == BUILTINSXP) {
	    deparse2buff(CAR(s), d);
	    print2buff("(", d);
	    args2buff(CDR(s), 0, 0, d);
	    print2buff(")", d);
	}
	else { /* we have a lambda expression */
	    deparse2buff(CAR(s), d);
	    print2buff("(", d);
	    args2buff(CDR(s), 0, 0, d);
	    print2buff(")", d);
	}
	if (localOpts & QUOTEEXPRESSIONS) {
	    d->opts = localOpts;
	    print2buff(")", d);
	}
	break;
    case STRSXP:
    case LGLSXP:
    case INTSXP:
    case REALSXP:
    case CPLXSXP:
    case RAWSXP:
	if (localOpts & SHOWATTRIBUTES) attr1(s, d);
	vector2buff(s, d);
	if (localOpts & SHOWATTRIBUTES) attr2(s, d);
	break;
    case EXTPTRSXP:
    	d->sourceable = FALSE;
	sprintf(tpb, "<pointer: %p>", R_ExternalPtrAddr(s));
	print2buff(tpb, d);
	break;
#ifdef BYTECODE
    case BCODESXP:
    	d->sourceable = FALSE;
	print2buff("<bytecode>", d);
	break;
#endif
    case WEAKREFSXP:
    	d->sourceable = FALSE;
	sprintf(tpb, "<weak reference>");
	print2buff(tpb, d);
	break;
    default:
    	d->sourceable = FALSE;
	UNIMPLEMENTED_TYPE("deparse2buff", s);
    }
}


/* If there is a string array active point to that, and */
/* otherwise we are counting lines so don't do anything. */

static void writeline(LocalParseData *d)
{
    if (d->strvec != R_NilValue)
	SET_STRING_ELT(d->strvec, d->linenumber, mkChar(d->buffer.data));
    d->linenumber++;
    /* reset */
    d->len = 0;
    d->buffer.data[0] = '\0';
    d->startline = TRUE;
}

static void print2buff(char *strng, LocalParseData *d)
{
    int tlen, bufflen;

    if (d->startline) {
	d->startline = FALSE;
	printtab2buff(d->indent, d);	/*if at the start of a line tab over */
    }
    tlen = strlen(strng);
    R_AllocStringBuffer(0, &(d->buffer));
    bufflen = strlen(d->buffer.data);
    /*if (bufflen + tlen > BUFSIZE) {
	buff[0] = '\0';
	error("string too long in deparse");
	}*/
    R_AllocStringBuffer(bufflen + tlen, &(d->buffer));
    strcat(d->buffer.data, strng);
    d->len += tlen;
}

static void scalar2buff(SEXP inscalar, LocalParseData *d)
{
    char *strp;
    strp = EncodeElement(inscalar, 0, '"');
    print2buff(strp, d);
}

static void vector2buff(SEXP vector, LocalParseData *d)
{
    int tlen, i, quote;
    char *strp;

    tlen = length(vector);
    if( isString(vector) )
	quote='"';
    else
	quote=0;
    if (tlen == 0) {
	switch(TYPEOF(vector)) {
	case LGLSXP: print2buff("logical(0)", d); break;
	case INTSXP: print2buff("integer(0)", d); break;
	case REALSXP: print2buff("numeric(0)", d); break;
	case CPLXSXP: print2buff("complex(0)", d); break;
	case STRSXP: print2buff("character(0)", d); break;
	case RAWSXP: print2buff("raw(0)", d); break;
	default: UNIMPLEMENTED_TYPE("vector2buff", vector);
	}
    }
    else if (tlen == 1) {
	if((d->opts & KEEPINTEGER) && TYPEOF(vector) == INTSXP) print2buff("as.integer(", d);
	scalar2buff(vector, d);
	if((d->opts & KEEPINTEGER) && TYPEOF(vector) == INTSXP) print2buff(")", d);
    }
    else {
	if((d->opts & KEEPINTEGER) && TYPEOF(vector) == INTSXP) print2buff("as.integer(", d);
	print2buff("c(", d);
	for (i = 0; i < tlen; i++) {
	    strp = EncodeElement(vector, i, quote);
	    print2buff(strp, d);
	    if (i < (tlen - 1))
		print2buff(", ", d);
	    if (d->len > d->cutoff)
		writeline(d);
	}
	print2buff(")", d);
	if((d->opts & KEEPINTEGER) && TYPEOF(vector) == INTSXP) print2buff(")", d);
    }

}

/* vec2buff : New Code */
/* Deparse vectors of S-expressions. */
/* In particular, this deparses objects of mode expression. */

static void vec2buff(SEXP v, LocalParseData *d)
{
    SEXP nv;
    int i, n;
    Rboolean lbreak = FALSE;
    Rboolean localOpts = d->opts;

    n = length(v);
    nv = getAttrib(v, R_NamesSymbol);
    if (length(nv) == 0) nv = R_NilValue;

    for(i = 0 ; i < n ; i++) {
	if (i > 0)
	    print2buff(", ", d);
	linebreak(&lbreak, d);
	if (!isNull(nv) && !isNull(STRING_ELT(nv, i))
	    && *CHAR(STRING_ELT(nv, i))) {
            d->opts = SIMPLEDEPARSE;
	    if( isValidName(CHAR(STRING_ELT(nv, i))) )
		deparse2buff(STRING_ELT(nv, i), d);
	    else {
		print2buff("\"", d);
		deparse2buff(STRING_ELT(nv, i), d);
		print2buff("\"", d);
	    }
	    d->opts = localOpts;
	    print2buff(" = ", d);
	}
	deparse2buff(VECTOR_ELT(v, i), d);
    }
    if (lbreak)
	d->indent--;
}

static void args2buff(SEXP arglist, int lineb, int formals, LocalParseData *d)
{
    Rboolean lbreak = FALSE;

    while (arglist != R_NilValue) {
	if (TYPEOF(arglist) != LISTSXP && TYPEOF(arglist) != LANGSXP)
            error(_("badly formed function expression"));
	if (TAG(arglist) != R_NilValue) {
#if 0
	    deparse2buff(TAG(arglist));
#else
	    char tpb[120];
	    SEXP s = TAG(arglist);

	    if( s == R_DotsSymbol || isValidName(CHAR(PRINTNAME(s))) )
		print2buff(CHAR(PRINTNAME(s)), d);
	    else {
		if( strlen(CHAR(PRINTNAME(s)))< 117 ) {
		    snprintf(tpb, 120, "\"%s\"",CHAR(PRINTNAME(s)));
		    print2buff(tpb, d);
		}
		else {
		    sprintf(tpb,"\"");
		    strncat(tpb, CHAR(PRINTNAME(s)), 117);
		    strcat(tpb, "\"");
		    print2buff(tpb, d);
		}
	    }
#endif
	    if(formals) {
		if (CAR(arglist) != R_MissingArg) {
		    print2buff(" = ", d);
		    deparse2buff(CAR(arglist), d);
		}
	    }
	    else {
		print2buff(" = ", d);
		if (CAR(arglist) != R_MissingArg) {
		    deparse2buff(CAR(arglist), d);
		}
	    }
	}
	else deparse2buff(CAR(arglist), d);
	arglist = CDR(arglist);
	if (arglist != R_NilValue) {
	    print2buff(", ", d);
	    linebreak(&lbreak, d);
	}
    }
    if (lbreak)
	d->indent--;
}

/* This code controls indentation.  Used to follow the S style, */
/* (print 4 tabs and then start printing spaces only) but I */
/* modified it to be closer to emacs style (RI). */

static void printtab2buff(int ntab, LocalParseData *d)
{
    int i;

    for (i = 1; i <= ntab; i++)
	if (i <= 4)
	    print2buff("    ", d);
	else
	    print2buff("  ", d);
}

/* Copyright (c) 2003 John Garvin
 *
 * Preliminary version v06, July 11, 2003 
 *
 * Dynamically loads and runs the specified object file generated by
 * compiling the output of rcc. This is a temporary measure;
 * eventually you will be able to do the same thing just by running R
 * code that looks like source("foo.r").
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <IOStuff.h>
#include <Parse.h>
#include <Internal.h>
#include "rcc_lib.h"

extern int Rf_initEmbeddedR(int argc, char **argv);

int main(int argc, char *argv[]) {
  SEXP v0, v1, v2, v3;
  char *myargv[] = {argv[0], "--gui=none", "--slave"};
  if (argc != 2) {
    fprintf(stderr, "Usage: run <name of .so file>\n");
    exit(1);
  }
  Rf_initEmbeddedR(3, myargv);
  v0 = PROTECT(mkPRIMSXP(359,0));
  v1 = PROTECT(mkString(argv[1]));
  v2 = PROTECT(ScalarLogical(1));
  v3 = PROTECT(cons(v1,cons(v2,cons(v2,R_NilValue))));
  do_dynload(R_NilValue,
	     v0,
	     v3,
	     R_GlobalEnv);
  UNPROTECT(4);
  return 0;
}

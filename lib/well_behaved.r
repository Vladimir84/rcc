# This file defines R functions used to communicate unevaluated
# logical expressions to the rcc compiler.

.rcc.assert     <- function(...) {}
.rcc.assert.sym <- function(...) {}
.rcc.assert.exp <- function(x, ...) x

# Assertions that guarantee "well-behaved" conditions

.rcc.assert(no.oo)
.rcc.assert(no.envir.manip)
.rcc.assert(no.special.redef)
.rcc.assert(no.builtin.redef)
.rcc.assert(no.library.redef)

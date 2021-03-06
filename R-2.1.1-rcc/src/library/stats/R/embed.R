# Copyright (C) 1997-1999  Adrian Trapletti
#
# Rewritten to use R indexing (C) 1999 R Core Development Team
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

embed <- function (x, dimension = 1)
{
    if (is.matrix(x)) {
        n <- nrow(x)
        m <- ncol(x)
        if ((dimension < 1) | (dimension > n))
            stop ("wrong embedding dimension")
        y <- matrix(0.0, n - dimension + 1, dimension * m)
        for (i in (1:m))
            y[, seq(i, by = m,length = dimension)] <-
                Recall (as.vector(x[,i]), dimension)
        return (y)
    } else if (is.vector(x) || is.ts(x)) {
        n <- length (x)
        if ((dimension < 1) | (dimension > n))
            stop ("wrong embedding dimension")
        m <- n - dimension + 1
        return(matrix(x[1:m + rep(dimension:1, rep(m, dimension)) - 1], m))
    } else
        stop ("'x' is not a vector or matrix")
}

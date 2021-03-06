\name{on.exit}
\alias{on.exit}
\title{Function Exit Code}
\description{
  \code{on.exit} records the expression given as its argument as needing
  to be executed when the current function exits (either naturally or as
  the result of an error).  This is useful for resetting graphical
  parameters or performing other cleanup actions.

  If no expression is provided, i.e., the call is \code{on.exit()}, then
  the current \code{on.exit} code is removed.
}
\usage{
on.exit(expr, add = FALSE)
}
\arguments{
  \item{expr}{an expression to be executed.}
  \item{add}{if TRUE, add \code{expr} to be executed after any previously
    set expressions.}
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth \& Brooks/Cole.
}
\seealso{
  \code{\link{sys.on.exit}} to see the current expression.
}
\examples{
opar <- par(mai = c(1,1,1,1))
on.exit(par(opar))
\dontshow{par(opar)}
}
\keyword{programming}

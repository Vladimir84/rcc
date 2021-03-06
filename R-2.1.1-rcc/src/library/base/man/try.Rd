\name{try}
\alias{try}
\title{Try an Expression Allowing Error Recovery}
\description{
  \code{try} is a wrapper to run an expression that might fail and allow
  the user's code to handle error-recovery.
}
\usage{
try(expr, silent = FALSE)
}
\arguments{
  \item{expr}{an \R expression to try.}
  \item{silent}{logical: should the report of error messages be suppressed?}
}
\details{
  \code{try} evaluates an expression and traps any errors that occur
  during the evaluation.  \code{try} establishes a handler for
  errors that uses the default error handling protocol. It also
  establishes a \code{tryRestart} restart that can be used by
  \code{invokeRestart}.
}
\value{
  The value of the expression if \code{expr} is evaluated without error,
  but an invisible object of class \code{"try-error"} containing the
  error message if it fails. The normal error handling will print the
  same message unless \code{options("show.error.messages")} is false or
  the call includes \code{silent = TRUE}.
}
\seealso{
  \code{\link{options}} for setting error handlers and suppressing the
  printing of error messages;
  \code{\link{geterrmessage}} for retrieving the last error message.
  \code{tryCatch} provides another means of catching and handling
  errors.
}
\examples{
## this example will not work correctly in example(try), but
## it does work correctly if pasted in
options(show.error.messages = FALSE)
try(log("a"))
print(.Last.value)
options(show.error.messages = TRUE)

## alternatively,
print(try(log("a"), TRUE))

## run a simulation, keep only the results that worked.
set.seed(123)
x <- rnorm(50)
doit <- function(x)
{
    x <- sample(x, replace=TRUE)
    if(length(unique(x)) > 30) mean(x)
    else stop("too few unique points")
}
## alternative 1
res <- lapply(1:100, function(i) try(doit(x), TRUE))
## alternative 2
\dontrun{res <- vector("list", 100)
for(i in 1:100) res[[i]] <- try(doit(x), TRUE)}
unlist(res[sapply(res, function(x) !inherits(x, "try-error"))])
}
\keyword{programming}

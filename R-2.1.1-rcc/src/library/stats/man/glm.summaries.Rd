\name{glm.summaries}
\alias{family.glm}
\alias{residuals.glm}
\title{Accessing Generalized Linear Model Fits}
\usage{
\method{family}{glm}(object, \dots)

\method{residuals}{glm}(object, type = c("deviance", "pearson", "working",
                           "response", "partial"), \dots)
}
\arguments{
  \item{object}{an object of class \code{glm}, typically the result of
    a call to \code{\link{glm}}.}
  \item{type}{the type of residuals which should be returned.
    The alternatives are: \code{"deviance"} (default), \code{"pearson"},
    \code{"working"}, \code{"response"}, and \code{"partial"}.}
  \item{\dots}{further arguments passed to or from other methods.}
}
\description{
  These functions are all \code{\link{methods}} for class \code{glm} or
  \code{summary.glm} objects.
}
\details{
  The references define the types of residuals: Davison \& Snell is a
  good reference for the usages of each.
  
  The partial residuals are a matrix of working residuals, with each
  column formed by omitting a term from the model.
}
\seealso{
  \code{\link{glm}} for computing \code{glm.obj}, \code{\link{anova.glm}};
  the corresponding \emph{generic} functions, \code{\link{summary.glm}},
  \code{\link{coef}}, \code{\link{deviance}},
  \code{\link{df.residual}},
  \code{\link{effects}}, \code{\link{fitted}},
  \code{\link{residuals}}.
}
\references{
  Davison, A. C. and Snell, E. J. (1991)
  \emph{Residuals and diagnostics.}  In: Statistical Theory
  and Modelling. In Honour of Sir David Cox, FRS, eds.
  Hinkley, D. V., Reid, N. and Snell, E. J., Chapman \& Hall.
  
  Hastie, T. J. and Pregibon, D. (1992)
  \emph{Generalized linear models.}
  Chapter 6 of \emph{Statistical Models in S}
  eds J. M. Chambers and T. J. Hastie, Wadsworth \& Brooks/Cole.

  McCullagh P. and Nelder, J. A. (1989)
  \emph{Generalized Linear Models.}
  London: Chapman and Hall.
}
\keyword{models}
\keyword{regression}


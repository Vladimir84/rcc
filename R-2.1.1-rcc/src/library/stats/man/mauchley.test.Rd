\name{mauchley.test}
\alias{mauchley.test}
\alias{mauchley.test.SSD}
\alias{mauchley.test.mlm}
\title{Mauchley's test of sphericity}
\description{
  Tests whether a Wishart-distributed covariance matrix (or
  transformation thereof) is
  proportional to a given matrix.
}
\usage{
## S3 methods for class 'SSD' or 'mlm' 
mauchley.test(object, Sigma = diag(nrow = p),
   T = Thin.row(proj(M) - proj(X)), M = diag(nrow = p), X = ~0,
   idata = data.frame(index = seq(length = p)), ...)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{object}{object of class \code{SSD} or \code{mlm}}
  \item{Sigma}{Matrix to be proportional to}
  \item{T}{Transformation matrix. By default computed from \code{M} and
    \code{X}}
  \item{M}{Formula or matrix describing the outer projection (see below)}
  \item{X}{Formula or matrix describing the inner projection (see below)}
  \item{idata}{Data frame describing intra-block design}
  \item{...}{For consistency with generic}
}
\details{
  Mauchley's test test for whether a covariance matrix can be assumed to
  be proportional to a given matrix. 
  
  It is common to transform the observations prior to testing. This
  typically involves 
  transformation to intra-block differences, but more complicated
  within-block designs can be encountered,
  making more elaborate transformations necessary. A
  transformation matrix \code{T} can be given directly or specified as
  the difference between two projections onto the spaces spanned by
  \code{M} and \code{X}, which in turn can be given as matrices or as
  model formulas with respect to \code{idata} (the tests will be
  invariant to parametrization of the quotient space \code{M/X}).

  The common use of this test is in repeated measurements designs, with
  \code{X=~1}. This is almost, but not quite the same as testing for
  compund symmetry in the untransformed covariance matrix.
}
\value{An object of class \code{"htest"}} %% perhaps elaborate?

\references{TW Anderson (1958). An Introduction to Multivariate
Statistical Analysis. Wiley}

\seealso{\code{\link{SSD}}, \code{\link{anova.mlm}}}

\note{The p-value differs slightly from that of SAS because a second order term
  is included in the asymptotic approximation.}

%% Probably use example from Baron/Li
\examples{
example(SSD) # Brings in the mlmfit and reacttime objects

### traditional test of intrasubj. contrasts
mauchley.test(mlmfit, X=~1) 

### tests using intra-subject 3x2 design
idata <- data.frame(deg=gl(3,1,6, labels=c(0,4,8)),
                    noise=gl(2,3,6, labels=c("A","P")))
mauchley.test(mlmfit, X = ~ deg + noise, idata = idata)
mauchley.test(mlmfit, M = ~ deg + noise, X = ~ noise, idata=idata)
}
\keyword{htest}
\keyword{models}
\keyword{multivariate}


\name{ts.plot}
\alias{ts.plot}
\title{Plot Multiple Time Series}
\usage{
ts.plot(\dots, gpars = list())
}
\arguments{
  \item{\dots}{one or more univariate or multivariate time series.}
  \item{gpars}{list of named graphics parameters to be passed to the
    plotting functions.  Those commonly used can be supplied directly in
    \code{\dots}.}
}
\description{
  Plot several time series on a common plot. Unlike
  \code{\link{plot.ts}} the series can have a different time bases,
  but they should have the same frequency.
}
\note{
  Although this can be used for a single time series, \code{plot} is
  easier to use and is preferred.
}
\value{
  None.
}

\seealso{\code{\link{plot.ts}}}

\examples{
ts.plot(ldeaths, mdeaths, fdeaths,
        gpars=list(xlab="year", ylab="deaths", lty=c(1:3)))
}
\keyword{ts}

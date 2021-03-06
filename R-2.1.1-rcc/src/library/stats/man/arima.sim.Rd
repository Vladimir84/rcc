\name{arima.sim}
\alias{arima.sim}
\title{Simulate from an ARIMA Model}
\description{
  Simulate from an ARIMA model.
}
\usage{
arima.sim(model, n, rand.gen = rnorm, innov = rand.gen(n, \dots),
          n.start = NA, \dots)
}
\arguments{
  \item{model}{A list with component \code{ar} and/or \code{ma} giving
    the AR and MA coefficients respectively.  Optionally a component
    \code{order} can be used. An empty list gives an ARIMA(0, 0, 0)
    model, that is white noise.}
  \item{n}{length of output series, before un-differencing.}
  \item{rand.gen}{optional: a function to generate the innovations.}
  \item{innov}{an optional times series of innovations.  If not
    provided, \code{rand.gen} is used.}
  \item{n.start}{length of \dQuote{burn-in} period.  If \code{NA}, the
    default, a reasonable value is computed.}
  \item{\dots}{additional arguments for \code{rand.gen}.  Most usefully,
    the standard deviation of the innovations generated by \code{rnorm}
    can be specified by \code{sd}.}
}
\details{
  See \code{\link{arima}} for the precise definition of an ARIMA model.

  The ARMA model is checked for stationarity.

  ARIMA models are specified via the \code{order} component of
  \code{model}, in the same way as for \code{\link{arima}}.  Other
  aspects of the \code{order} component are ignored, but inconsistent
  specifications of the MA and AR orders are detected.  The
  un-differencing assumes previous values of zero, and to remind the
  user of this, those values are returned.
}
\value{
  A time-series object of class \code{"ts"}.
}
\seealso{
  \code{\link{arima}}
}
\examples{
arima.sim(n = 63, list(ar = c(0.8897, -0.4858), ma = c(-0.2279, 0.2488)),
          sd = sqrt(0.1796))
# mildly long-tailed
arima.sim(n = 63, list(ar=c(0.8897, -0.4858), ma=c(-0.2279, 0.2488)),
          rand.gen = function(n, ...) sqrt(0.1796) * rt(n, df = 5))

# An ARIMA simulation
ts.sim <- arima.sim(list(order = c(1,1,0), ar = 0.7), n = 200)
ts.plot(ts.sim)
}
\keyword{ts}

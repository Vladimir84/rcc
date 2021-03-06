\name{scatter.smooth}
\title{Scatter Plot with Smooth Curve Fitted by Loess}
\alias{scatter.smooth}
\alias{loess.smooth}
\description{
  Plot and add a smooth curve computed by \code{loess} to a scatter plot.
}
\usage{
scatter.smooth(x, y, span = 2/3, degree = 1,
    family = c("symmetric", "gaussian"),
    xlab = deparse(substitute(x)), ylab = deparse(substitute(y)),
    ylim = range(y, prediction$y, na.rm = TRUE),
    evaluation = 50, \dots)

loess.smooth(x, y, span = 2/3, degree = 1,
    family = c("symmetric", "gaussian"), evaluation = 50, \dots)
}
\arguments{
  \item{x}{x coordinates for scatter plot.}
  \item{y}{y coordinates for scatter plot.}
  \item{span}{smoothness parameter for \code{loess}.}
  \item{degree}{degree of local polynomial used.}
  \item{family}{if \code{"gaussian"} fitting is by least-squares, and if
    \code{family="symmetric"} a re-descending M estimator is used.} 
  \item{xlab}{label for x axis.}
  \item{ylab}{label for y axis.}
  \item{ylim}{the y limits of the plot.}
  \item{evaluation}{number of points at which to evaluate the smooth
    curve.}
  \item{\dots}{graphical parameters.}
}
\details{
  \code{loess.smooth} is an auxiliary function which evaluates the
  \code{loess} smooth at \code{evaluation} equally spaced points
  covering the range of \code{x}.
}
\value{
  For \code{scatter.smooth}, none.

  For \code{loess.smooth}, a list with two components, \code{x} (the
  grid of evaluation points) and \code{y} (the smoothed values at the
  grid points).
}
\seealso{\code{\link{loess}}}
\examples{
attach(cars)
scatter.smooth(speed, dist)
detach()
}
\keyword{smooth}

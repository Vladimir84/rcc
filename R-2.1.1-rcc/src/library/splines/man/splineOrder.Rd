\name{splineOrder}
\alias{splineOrder}
%\alias{splineOrder.bSpline}
%\alias{splineOrder.polySpline}
\title{Determine the Order of a Spline}
\description{
  Return the order of a spline object.
}
\usage{
splineOrder(object)
}
\arguments{
 \item{object}{An object that inherits from class \code{"spline"}.}
}
\details{
  The order of a spline is the number of coefficients in each piece of
  the piecewise polynomial representation.  Thus a cubic spline has
  order 4.
}
\value{
  A positive integer.
}
\author{Douglas Bates and Bill Venables}
\seealso{
  \code{\link{splineKnots}},
  \code{\link{interpSpline}},
  \code{\link{periodicSpline}}
}
\examples{
splineOrder( interpSpline( weight ~ height, women ) )
}
\keyword{ models }

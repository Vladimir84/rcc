\name{asVector}
\alias{asVector}
%\alias{asVector.xyVector}
\title{Coerce an Object to a Vector}
\description{
  This is a generic function.  Methods for this function coerce objects
  of given classes to vectors.
}
\usage{
asVector(object)
}
\arguments{
  \item{object}{An object.}
}
\value{
  a vector
}
\details{
  Methods for vector coercion in new classes must be created for the
  \code{asVector} generic instead of \code{as.vector}.  The
  \code{as.vector} function is internal and not easily extended.
  Currently the only class with an \code{asVector} method is the
  \code{xyVector} class.
}
\author{Douglas Bates and Bill Venables}
\seealso{
  \code{\link{xyVector}}
}
\examples{
ispl <- interpSpline( weight ~ height,  women )
pred <- predict(ispl)
class(pred)
utils::str(pred)
asVector(pred)
}
\keyword{models}

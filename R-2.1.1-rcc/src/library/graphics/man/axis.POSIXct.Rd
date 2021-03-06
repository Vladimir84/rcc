\name{axis.POSIXct}
\alias{axis.POSIXct}
\alias{plot.POSIXct}
\alias{plot.POSIXlt}
\alias{axis.Date}
\alias{plot.Date}
\title{Date and Date-time Plotting Functions}
\description{
  Functions to plot objects of classes \code{"POSIXlt"},
  \code{"POSIXct"} and \code{"Date"}  representing calendar dates and times. 
}
\usage{
axis.POSIXct(side, x, at, format, \dots)

axis.Date(side, x, at, format, \dots)

\method{plot}{POSIXct}(x, y, xlab = "", \dots)

\method{plot}{POSIXlt}(x, y, xlab = "", \dots)

\method{plot}{Date}(x, y, xlab = "", \dots)
}
\synopsis{
plot.POSIXct(x, y, xlab = "", axes = TRUE, frame.plot = axes, xaxt = par("xaxt"), ...)
plot.POSIXlt(x, y, xlab = "", axes = TRUE, frame.plot = axes, xaxt = par("xaxt"), ...)
plot.Date(x, y, xlab = "", axes = TRUE, frame.plot = axes, xaxt = par("xaxt"), ...)
axis.POSIXct(side, x, at, format, ...)
axis.Date(side, x, at, format, ...)
}
\arguments{
  \item{x, at}{A date-time object.}
  \item{y}{numeric values to be plotted against \code{x}.}
  \item{xlab}{a character string giving the label for the x axis.}
  \item{side}{See \code{\link{axis}}.}
  \item{format}{See \code{\link{strptime}}.}
  \item{\dots}{Further arguments to be passed from or to other methods,
    typically graphical parameters or arguments of
    \code{\link{plot.default}}.}
}
\details{
  The functions plot against an x-axis of date-times.
  \code{axis.POSIXct} and \code{axis.Date} work quite hard to choose
  suitable time units (years, months, days, hours, minutes or seconds)
  and a sensible output format, but this can be overridden by supplying
  a \code{format} specification.

  If \code{at} is supplied it specifies the locations of the ticks
  and labels whereas if \code{x} is specified a suitable grid of labels
  is chosen. 
}
\seealso{
  \link{DateTimeClasses}, \link{Dates} for details of the classes.
}
\examples{
attach(beaver1)
time <- strptime(paste(1990, day, time \%/\% 100, time \%\% 100),
                 "\%Y \%j \%H \%M")
plot(time, temp, type="l") # axis at 4-hour intervals.
# now label every hour on the time axis
plot(time, temp, type="l", xaxt="n")
r <- as.POSIXct(round(range(time), "hours"))
axis.POSIXct(1, at=seq(r[1], r[2], by="hour"), format="\%H")
rm(time)
detach(beaver1)

plot(.leap.seconds, 1:22, type="n", yaxt="n",
     xlab="leap seconds", ylab="", bty="n")
rug(.leap.seconds)
## or as dates
lps <- as.Date(.leap.seconds)
plot(lps, 1:22, type="n", yaxt="n", xlab="leap seconds", ylab="", bty="n")
rug(lps)

## 100 random dates in a 10-week period
random.dates <- as.Date("2001/1/1") + 70*sort(runif(100))
plot(random.dates, 1:100)
# or for a better axis labelling
plot(random.dates, 1:100, xaxt="n")
axis.Date(1, at=seq(as.Date("2001/1/1"), max(random.dates)+6, "weeks"))
}
\keyword{utilities}
\keyword{chron}

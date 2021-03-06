\name{Sys.sleep}
\alias{Sys.sleep}
\title{Suspend Execution for a Time Interval}
\description{
  Suspend execution of \R expressions for a given number of seconds
}
\usage{
Sys.sleep(time)
}
\arguments{
  \item{time}{The time interval to suspend execution for, in seconds.}
}
\details{
  Using this function allows \R to be given very low priority and hence
  not to interfere with more important foreground tasks. A typical use
  is to allow a process launched from \R to set itself up and read its
  input files before \R execution is resumed.
  
  The intention is that this function suspends execution of \R
  expressions but wakes the process up often enough to respond to GUI
  events, typically every 0.5 seconds.

  There is no guarantee that the process will sleep for the whole of the
  specified interval, and it may well take slightly longer in real time
  to resume execution.  The resolution of the time interval is
  system-dependent, but will normally be down to 0.02 secs or better.
}
\value{
  Invisible \code{NULL}.
}
\note{This function may not be implemented on all systems.}

\examples{
testit <- function(x)
{
    p1 <- proc.time()
    Sys.sleep(x)
    proc.time() - p1 # The cpu usage should be negligible
}
testit(3.7)
}
\keyword{utilities}

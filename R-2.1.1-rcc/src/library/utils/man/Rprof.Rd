\name{Rprof}
\alias{Rprof}
\title{Enable Profiling of R's Execution}
\description{
  Enable or disable profiling of the execution of \R expressions.
}
\usage{
Rprof(filename = "Rprof.out", append = FALSE, interval = 0.02)
}
\arguments{
  \item{filename}{
    The file to be used for recording the profiling results.
    Set to \code{NULL} or \code{""} to disable profiling.
  }
  \item{append}{
    logical: should the file be over-written or appended to?
  }
  \item{interval}{
    real: time interval between samples.
  }
}
\details{
  Enabling profiling automatically disables any existing profiling to
  another or the same file.
  
  Profiling works by writing out the call stack every \code{interval}
  seconds, to the file specified. Either the \code{\link{summaryRprof}}
  function or the Perl script \code{R CMD Rprof} can be used to process
  the output file to produce a summary of the
  usage; use \code{R CMD Rprof --help} for usage information.

#ifdef windows
  Exactly what the time interval measures is subtle: it is time that the
  \R process is running and executing an \R command.  It is not however just
  CPU time, for if \code{readline()} is waiting for input, that counts
  (on Windows, but not on Unix).

  Note that the timing interval cannot be too small, for the time spent
  in each profiling step is added to the interval.  What is feasible is
  machine-dependent, but 10ms seems as small as advisable on a 1GHz machine.
#endif
#ifdef unix
  Note that the timing interval cannot be too small: once the timer goes
  off, the information is not recorded until the next clock tick
  (probably every 10msecs).  Thus the interval is rounded to the nearest
  integer number of clock ticks, and is made to be at least one clock
  tick (at which resolution the total time spent is liable to be
  underestimated).
#endif
}
\note{
#ifdef windows
  Using \code{R CMD Rprof} needs Windows Perl to be installed.
#endif
#ifdef unix
  Profiling is not available on all platforms.  By default, it is
  attempted to compile support for profiling.  Configure \R with
  \option{--disable-R-profiling} to change this.

  As \R profiling uses the same mechanisms as C profiling, the two
  cannot be used together, so do not use \code{Rprof} in an executable
  built for profiling.
#endif
}
\seealso{
  The chapter on \dQuote{Tidying and profiling R code} in
  \dQuote{Writing \R Extensions} (see the \file{doc/manual} subdirectory
  of the \R source tree).

  \code{\link{summaryRprof}}
}
\examples{
\dontrun{Rprof()
## some code to be profiled
Rprof(NULL)
## some code NOT to be profiled
Rprof(append=TRUE)
## some code to be profiled
Rprof(NULL)
\dots
## Now post-process the output as described in Details
}}
\keyword{utilities}

\name{MethodSupport}
\alias{.conflicts.OK}
\alias{OldEvalSelectedMethod}
\alias{getMethodsForDispatch}
\alias{cacheMethod}
\alias{resetGeneric}
\title{Additional (Support) Functions for Methods}
\description{
  These are support routines for computations on formal methods.
}
\usage{
getMethodsForDispatch(f, fdef)

cacheMethod(f, sig, def, args, fdef)

resetGeneric(f, fdef, mlist, where, deflt)
}
\section{Summary of Functions}{
  \describe{
    \item{\code{resetGeneric}:}{ 
      reset the currently defined methods for this generic to the
      currently visible methods, looking from environment \code{where}.  Returns \code{TRUE}
      or \code{FALSE} according to whether information for the function
      was found in the metadata.

      Normally not called directly, since changes to methods,
    attaching and detaching packages all generate a call automatically.
    }
    \item{\code{cacheMethod}:}{
      Store the definition for this function and signature in the method
      metadata for the function.  Used to store extensions of coerce
      methods found through inheritance.

      No persistent effect, since the method metadata is session-scope
      only.
    }
    \item{\code{getMethodsForDispatch}:}{
      Get the current methods list object representing the methods for
      function \code{f}, merged from the various packages and with any
      additional caching information stored in the \code{allMethods}
      slot.

      If methods have not yet been merged, calling
      \code{getMethodsForDispatch} will cause the merge to take place.
    }
  }
}
\keyword{internal}

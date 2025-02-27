% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/BFmodel.R, R/BFpop.R
\name{BFmodel}
\alias{BFmodel}
\title{Main interface class for BLOFELD models}
\description{
TODO

Population classes must derive from BFpop and override the following methods:
\itemize{
\item reset
\item update (with time step; also resets ext_beta to zero)
\item state (active binding)
\item check (validation)
\item ext_beta (double)
\item ext_infect (double/int of number to convert to infected)
\item IsCpp (optionally; should be TRUE if the class is implemented in C++)
}
}
\section{Active bindings}{
\if{html}{\out{<div class="r6-active-bindings">}}
\describe{
\item{\code{beta_matrix}}{a matrix representing between-group (density-dependent) spread}

\item{\code{groups}}{a list of the internally-stored within-group models (this can be used to directly check and/or modify their state and/or parameters)}

\item{\code{time}}{the current time point}

\item{\code{transmission_between}}{the between-group transmission type (frequency or density)}

\item{\code{state}}{a data frame of the current state of each group}
}
\if{html}{\out{</div>}}
}
\section{Methods}{
\subsection{Public methods}{
\itemize{
\item \href{#method-BFmodel-new}{\code{BFmodel$new()}}
\item \href{#method-BFmodel-update}{\code{BFmodel$update()}}
\item \href{#method-BFmodel-save}{\code{BFmodel$save()}}
\item \href{#method-BFmodel-reset}{\code{BFmodel$reset()}}
\item \href{#method-BFmodel-run}{\code{BFmodel$run()}}
\item \href{#method-BFmodel-run_until}{\code{BFmodel$run_until()}}
\item \href{#method-BFmodel-print}{\code{BFmodel$print()}}
\item \href{#method-BFmodel-clone}{\code{BFmodel$clone()}}
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-new"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-new}{}}}
\subsection{Method \code{new()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$new(bf_control, bf_pop = list(), bf_spread = list())}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-update"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-update}{}}}
\subsection{Method \code{update()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$update(d_time)}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-save"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-save}{}}}
\subsection{Method \code{save()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$save()}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-reset"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-reset}{}}}
\subsection{Method \code{reset()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$reset()}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-run"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-run}{}}}
\subsection{Method \code{run()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$run(add_time, d_time)}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-run_until"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-run_until}{}}}
\subsection{Method \code{run_until()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$run_until(criterion_fun, d_time)}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-print"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-print}{}}}
\subsection{Method \code{print()}}{
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$print()}\if{html}{\out{</div>}}
}

}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-clone"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-clone}{}}}
\subsection{Method \code{clone()}}{
The objects of this class are cloneable with this method.
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$clone(deep = FALSE)}\if{html}{\out{</div>}}
}

\subsection{Arguments}{
\if{html}{\out{<div class="arguments">}}
\describe{
\item{\code{deep}}{Whether to make a deep clone.}
}
\if{html}{\out{</div>}}
}
}
}
\section{Active bindings}{
\if{html}{\out{<div class="r6-active-bindings">}}
\describe{
\item{\code{beta_matrix}}{a matrix representing between-group (density-dependent) spread}

\item{\code{groups}}{a list of the internally-stored within-group models (this can be used to directly check and/or modify their state and/or parameters)}

\item{\code{time}}{the current time point}

\item{\code{transmission_between}}{the between-group transmission type (frequency or density)}

\item{\code{state}}{a data frame of the current state of each group}
}
\if{html}{\out{</div>}}
}
\section{Methods}{
\subsection{Public methods}{
\itemize{
\item \href{#method-BFmodel-new}{\code{BFmodel$new()}}
\item \href{#method-BFmodel-update}{\code{BFmodel$update()}}
\item \href{#method-BFmodel-save}{\code{BFmodel$save()}}
\item \href{#method-BFmodel-reset}{\code{BFmodel$reset()}}
\item \href{#method-BFmodel-run}{\code{BFmodel$run()}}
\item \href{#method-BFmodel-run_until}{\code{BFmodel$run_until()}}
\item \href{#method-BFmodel-print}{\code{BFmodel$print()}}
\item \href{#method-BFmodel-clone}{\code{BFmodel$clone()}}
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-new"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-new}{}}}
\subsection{Method \code{new()}}{
Create a new BLOFELD model
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$new(bf_control, bf_pop = list(), bf_spread = list())}\if{html}{\out{</div>}}
}

\subsection{Arguments}{
\if{html}{\out{<div class="arguments">}}
\describe{
\item{\code{bf_control}}{a single object of (or deriving from) class BFcontrol}

\item{\code{bf_pop}}{a list of one or more object(s) of (or deriving from) class BFpop}

\item{\code{bf_spread}}{a list of zero or more object(s) of (or deriving from) class BFspread}

\item{\code{bf_report}}{a list of zero or more object(s) of (or deriving from) class BFreport}
}
\if{html}{\out{</div>}}
}
\subsection{Returns}{
A new BLOFELD model object
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-update"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-update}{}}}
\subsection{Method \code{update()}}{
Update the state of each group for a single time point, including
setting trans_external based on the value of the beta_matrix and vector
of I obtained from the groups (density-dependent spread).
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$update(d_time)}\if{html}{\out{</div>}}
}

\subsection{Arguments}{
\if{html}{\out{<div class="arguments">}}
\describe{
\item{\code{d_time}}{the desired time step (delta time)}
}
\if{html}{\out{</div>}}
}
\subsection{Returns}{
self, invisibly
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-save"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-save}{}}}
\subsection{Method \code{save()}}{
Instruct each group to save the current state and parameter values for later retrieval using reset()
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$save()}\if{html}{\out{</div>}}
}

\subsection{Returns}{
self, invisibly
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-reset"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-reset}{}}}
\subsection{Method \code{reset()}}{
Instruct each group to reset the current state and parameter values to their last saved state
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$reset()}\if{html}{\out{</div>}}
}

\subsection{Returns}{
self, invisibly
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-run"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-run}{}}}
\subsection{Method \code{run()}}{
Update the state of each group for a given number of time points
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$run(add_time, d_time)}\if{html}{\out{</div>}}
}

\subsection{Arguments}{
\if{html}{\out{<div class="arguments">}}
\describe{
\item{\code{add_time}}{the additional time to add to the current time of the model}

\item{\code{d_time}}{the desired time step (delta time)}
}
\if{html}{\out{</div>}}
}
\subsection{Returns}{
a data frame of the model state at each (new) time point
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-run_until"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-run_until}{}}}
\subsection{Method \code{run_until()}}{
Update the state of each group for several time points, until a stopping
criterion is met.
NOTE: this method is a stub: it has not yet been implemented!
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$run_until(criterion_fun, d_time)}\if{html}{\out{</div>}}
}

\subsection{Arguments}{
\if{html}{\out{<div class="arguments">}}
\describe{
\item{\code{criterion_fun}}{a function taking an input data frame of states and returning a logical scalar indicating if the simulation should be stopped (TRUE) or continue to be
updated (FALSE)}

\item{\code{d_time}}{the desired time step (delta time)}
}
\if{html}{\out{</div>}}
}
\subsection{Returns}{
a data frame of the model state at each (new) time point
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-print"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-print}{}}}
\subsection{Method \code{print()}}{
Print method showing the number of groups and current time
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$print()}\if{html}{\out{</div>}}
}

\subsection{Returns}{
self, invisibly
}
}
\if{html}{\out{<hr>}}
\if{html}{\out{<a id="method-BFmodel-clone"></a>}}
\if{latex}{\out{\hypertarget{method-BFmodel-clone}{}}}
\subsection{Method \code{clone()}}{
The objects of this class are cloneable with this method.
\subsection{Usage}{
\if{html}{\out{<div class="r">}}\preformatted{BFmodel$clone(deep = FALSE)}\if{html}{\out{</div>}}
}

\subsection{Arguments}{
\if{html}{\out{<div class="arguments">}}
\describe{
\item{\code{deep}}{Whether to make a deep clone.}
}
\if{html}{\out{</div>}}
}
}
}

if(length(find("mlists"))==0L) source("R/R6utils.R")

#' Template population class for BLOFELD models
#'
#' @description
#' Population classes must derive from BFpop and override the following methods:
#' - reset
#' - update (with time step; also resets ext_beta to zero)
#' - state (active binding)
#' - check (validation)
#' - ext_beta (double)
#' - ext_infect (double/int of number to convert to infected)
#' - IsCpp (optionally; should be TRUE if the class is implemented in C++)
#'
#' @importFrom tibble tibble
#' @importFrom dplyr bind_rows
#'
#' @export
BFmodel <- R6::R6Class(

  "BFmodel",

  public = mlist(

    #' @description
    #' Create a new BLOFELD model
    #' @param bf_pop a list of one or more object(s) of (or deriving from) class BFpop
    #' @param bf_spread a list of zero or more object(s) of (or deriving from) class BFspread
    #' @param bf_report a list of zero or more object(s) of (or deriving from) class BFreport
    #' @param bf_control a single object of (or deriving from) class BFcontrol
    #'
    #' @return A new BLOFELD model object
    initialize = function(bf_control, bf_pop = list(), bf_spread = list()){

      ## Allow a single pop and/or single spread as non-list:
      if(!inherits(bf_pop, "list")){
        bf_pop <- list(bf_pop)
      }
      if(!inherits(bf_spread, "list")){
        bf_spread <- list(bf_spread)
      }

      ## TODO: check classes and interfaces

      if(!is.list(groups) || length(list)==0L || any(!sapply(groups, \(x) inherits(x, "R6")))){
        # TODO: fails for C++
        #stop("The input models must be a list of 1 or more R6 objects")
      }
      if(!all(sapply(groups, \(x){
        all(c("I","state","trans_external","update") %in% names(x)) &&
          length(formals(x$update))==1L
      }))){
        # TODO: fails for C++
        #stop("All input models must have public fields (or active bindings) for I, state and trans_external, as well as a public update method with a single argument (d_time)")
      }

      ## TODO: set value correctly
      private$.allcpp <- FALSE

      private$.groups <- groups
      private$.ngroups <- length(groups)

      private$.beta_matrix <- matrix(0, ncol=private$.ngroups, nrow=private$.ngroups)
      private$.time <- 0

      self$save()

    },

    #' @description
    #' Update the state of each group for a single time point, including
    #' setting trans_external based on the value of the beta_matrix and vector
    #' of I obtained from the groups (density-dependent spread).
    #' @param d_time the desired time step (delta time)
    #' @return self, invisibly
    update = function(d_time){

      qassert(d_time, "N1(0,)")
      stopifnot(dim(private$.beta_matrix)==private$.ngroups)

      ## TODO: C++ function if private$.allcpp

      if(private$.trans_between == "frequency"){
        multby <- sapply(private$.groups, \(x){ x$I / x$N })
      }else if(private$.trans_between == "density"){
        multby <- sapply(private$.groups, \(x) x$I)
      }else{
        stop("Unrecognised private$.trans_between")
      }
      trans_b <- colSums(private$.beta_matrix * multby)
      for(i in seq_along(private$.groups)){
        private$.groups[[i]]$trans_external <- trans_b[i]
        private$.groups[[i]]$update(d_time)
      }

      invisible(self)
    },

    #' @description
    #' Instruct each group to save the current state and parameter values for later retrieval using reset()
    #' @return self, invisibly
    save = function(){
      lapply(private$.groups, \(x) x$save())
      private$.time <- 0
    },

    #' @description
    #' Instruct each group to reset the current state and parameter values to their last saved state
    #' @return self, invisibly
    reset = function(){
      lapply(private$.groups, \(x) x$reset())
      private$.time <- 0
    },

    #' @description
    #' Update the state of each group for a given number of time points
    #' @param add_time the additional time to add to the current time of the model
    #' @param d_time the desired time step (delta time)
    #' @return a data frame of the model state at each (new) time point
    run = function(add_time, d_time){

      qassert(add_time, "N1(0,)")
      qassert(d_time, "N1(0,)")
      stopifnot(dim(private$.beta_matrix)==private$.ngroups)

      ## TODO: C++ function if private$.allcpp

      c(
        if(self$time==0){
          list(self$state)
        }else{
          list()
        },
        lapply(seq(self$time+d_time, self$time+add_time, by=d_time), function(x){
          self$update(d_time)$state
        })
      ) |>
        bind_rows() ->
        out
      class(out) <- c("ipdmr_dm", class(out))
      attr(out, "ngroups") <- private$.ngroups
      out

    },

    #' @description
    #' Update the state of each group for several time points, until a stopping
    #' criterion is met.
    #' NOTE: this method is a stub: it has not yet been implemented!
    #' @param criterion_fun a function taking an input data frame of states and returning a logical scalar indicating if the simulation should be stopped (TRUE) or continue to be
    #' updated (FALSE)
    #' @param d_time the desired time step (delta time)
    #' @return a data frame of the model state at each (new) time point
    run_until = function(criterion_fun, d_time){
      stop("This method is not yet implemented")
      ## TODO: criterion should be a function accepting a data frame of states
      ## and returning a logical i.e. continue/stop
    },

    #' @description
    #' Print method showing the number of groups and current time
    #' @return self, invisibly
    print = function(){
      cat("A between-group model with ", private$.ngroups, " within-group models\n", sep="")
      cat("[Current time point = ", private$.time, "]", sep="")
      invisible(self)
    },

    .dummy=NULL
  ),

  private = mlist(

    .allcpp = FALSE,
    .beta_matrix = matrix(),
    .ngroups = numeric(),
    .groups = list(),
    .time = numeric(),
    .trans_between = "density",

    .dummy=NULL
  ),

  active = mlist(

    #' @field beta_matrix a matrix representing between-group (density-dependent) spread
    beta_matrix = function(value){
      if(missing(value)) return(private$.beta_matrix)
      stopifnot(value >= 0.0, dim(value)==private$.ngroups)
      private$.beta_matrix <- value
    },

    #' @field groups a list of the internally-stored within-group models (this can be used to directly check and/or modify their state and/or parameters)
    groups = function(){
      private$.groups
    },

    #' @field time the current time point
    time = function(){
      private$.time
    },

    #' @field transmission_between the between-group transmission type (frequency or density)
    transmission_between = function(value){
      if(missing(value)) return(private$.trans_between)
      value <- match.arg(value, c("frequency","density"))
      private$.trans_between <- value
    },

    #' @field state a data frame of the current state of each group
    state = function(){
      lapply(private$.groups, function(x) x$state) |>
        bind_rows() |>
        ## In case not all models have the same compartments:
        mutate(across(is.numeric, \(x) replace_na(x, 0))) |>
        mutate(GroupIndex = str_c("Gp", format(seq_along(private$.groups)) |> str_replace_all(" ", "0"))) |>
        select("GroupIndex", everything())
    },

    .dummy=NULL
  ),

  lock_class = TRUE
)


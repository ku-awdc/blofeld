#' Object-oriented implementation of a basic spread module
#'
#' @docType class
#' @importFrom R6 R6Class
#' @format [R6Class] object with fields and methods as documented below
#' @return [R6Class] object representing an agent-based model simulation
#'
#' @field Status The current status of the simulation
#'
#' @section Methods:
#' The following methods are available:
#' * `initialize()` the initialize method
#' * `print()` the print method
#'
#' @examples
#' bp <- BasicPop$new(50); bp
#' bp$set_extbeta(c(1, rep(0,49)))
#' bp$update(1)
#' bp$set_extbeta(c(0, rep(0,49)))
#' bp$update(99)
#' bp$compartments
#' @export

BasicRegulator <- R6Class("BasicRegulator", public = list(
  populations = list(), surveillance = list(), sensitivity = numeric(),
  specificity = numeric()
)
)

BasicRegulator$set("public", "initialize", function(population, sensitivity=0.5, specificity=0.99){

  self$populations <- list(population)
  self$surveillance <- list(observed_state = numeric(population$N))

  invisible(self)
})

BasicRegulator$set("public", "update", function(){

  stopifnot(length(self$populations) == 1)
  inf_farms <- self$populations[[1]]$compartments[1,"I",] > 0

  self$surveillance$observed_state[inf_farms] <- rbinom(sum(inf_farms), 1L, self$sensitivity)
  self$surveillance$observed_state[!inf_farms] <- rbinom(sum(!inf_farms), 1L, 1-self$specificity)

  self$surveillance$observed_state

  invisible(self)
})


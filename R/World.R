#' R World Class
#'
#' Worlds contain only data and no methods
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

## We initialise world with a list of factories for each
## population, which create the necessary data structures
World <- R6Class("World", public = list(
  population_factories = list(),
  spatial_factory = list(), temporal_factory = list()
  )
)

Simulation$set("public", "initialize", function(authority, populations, regulators, movements, indirect, logger){

  # Must have exactly 1:
  stopifnot(length(authority)==1L)
  stopifnot(length(logger)==1L)

  # Must have at least 1 population and regulator:
  stopifnot(length(populations)>=1L)
  stopifnot(length(regulators)>=1L)

  # May have 0 or more movements and indirects

  # TODO: check validity of provided classes
  # TODO: if inputs are provided as classes then make them a list

  self$authority <- authority
  self$populations <- populations
  self$regulators <- regulators
  self$movements <- movements
  self$indirects <- indirects
  self$logger <- logger

  invisible(self)
})

Simulation$set("public", "reset", function(date = as.Date("2020-01-01")){

  invisible(self)
})

Simulation$set("public", "update", function(iterations){

  invisible(self)
})


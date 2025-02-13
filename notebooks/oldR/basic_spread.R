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

BasicSpread <- R6Class("BasicSpread", public = list(
  network = matrix(), populations = list(), probability = numeric(),
  proportion = numeric()
)
)

BasicSpread$set("public", "initialize", function(population, probability = 0.1, proportion = 0.05){

  self$populations <- list(population)

  edges <- expand_grid(from = seq(1L,as.integer(population$num_farms),1L), nb=c(-1L,1L)) %>%
    mutate(to = from + nb) %>%
    mutate(to = case_when(
      to == 0 ~ max(from),
      to > max(from) ~ 1L,
      TRUE ~ to
    )) %>%
    select(-nb)

  self$network <- igraph::graph_from_data_frame(edges, directed=TRUE)

  self$probability <- probability
  self$proportion <- proportion

  invisible(self)
})

BasicSpread$set("public", "update", function(){

  stopifnot(length(self$populations) == 1)
  inf_farms <- which(self$populations[[1]]$compartments[1,"I",] > 0)

  inf_events <- self$network %>%
    as_data_frame() %>%
    filter(from %in% inf_farms) %>%
    filter(rbinom(n(), 1, self$probability) > 0)

  for(r in seq_len(nrow(inf_events))){

    self$populations[[1]]$move_animals(as.numeric(inf_events$from[r]), as.numeric(inf_events$to[r]), self$proportion)

  }

  invisible(self)
})


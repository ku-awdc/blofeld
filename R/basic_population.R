#' Object-oriented implementation of a basic population
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

BasicPop <- R6Class("BasicPop", public = list(
  compartments = array(), beta = 0.3, gamma = 0.1, ext_beta = numeric(), animal_groups = c("Animals"), disease_states = c("S","I","R"), num_farms = integer(), N = 100
)
)

BasicPop$set("public", "initialize", function(num_farms){

  self$compartments <- array(0, dim=c(1,3,num_farms), dimnames=list(self$animal_groups, self$disease_states, paste0("Farm_",1:num_farms)))
  self$compartments[1,1,] <- self$N
  self$num_farms <- num_farms

  invisible(self)
})

BasicPop$set("public", "set_extbeta", function(ext_beta){

  stopifnot(self$num_farms == length(ext_beta))
  self$ext_beta <- ext_beta

  invisible(self)
})

BasicPop$set("public", "move_animals", function(from, to, batch, move=FALSE){

  stopifnot(from <= self$num_farms, from > 0, from%%1 == 0)
  stopifnot(to <= self$num_farms, to > 0, to%%1 == 0)
  stopifnot(length(batch)==length(self$animal_groups))
  stopifnot(batch >= 0, batch <= 1)

  fromN <- sum(self$compartments[,,from])
  toN <- sum(self$compartments[,,to])

  mbatch <- self$compartments[,,from] * batch
  stopifnot(mbatch >= 0)
  self$compartments[,,from] <- self$compartments[,,from] - mbatch
  self$compartments[,,to] <- self$compartments[,,to] + mbatch

  if(!move){
    self$compartments[,,from] <- self$compartments[,,from] * (fromN / sum(self$compartments[,,from]))
    self$compartments[,,to] <- self$compartments[,,to] * (toN / sum(self$compartments[,,to]))

    stopifnot(isTRUE(all.equal(sum(self$compartments[,,from]), fromN)))
    stopifnot(isTRUE(all.equal(sum(self$compartments[,,to]), toN)))
  }

  stopifnot(self$compartments[1,"S",] >= 0)
  stopifnot(self$compartments[1,"I",] >= 0)
  stopifnot(self$compartments[1,"R",] >= 0)

  invisible(self)
})

BasicPop$set("public", "update", function(iterations=1){

  for(i in seq_len(iterations)){
    newI1 <- self$beta*self$compartments[1,"S",]*self$compartments[1,"I",]/self$N
    newI2 <- self$ext_beta*(self$compartments[1,"S",]-newI1)
    newI <- newI1 + newI2
    newR <- self$gamma*self$compartments[1,"I",]

    stopifnot(newI >= 0, newR >= 0)

    self$compartments[1,"S",] <- self$compartments[1,"S",] - newI
    self$compartments[1,"I",] <- self$compartments[1,"I",] + newI - newR
    self$compartments[1,"R",] <- self$compartments[1,"R",] + newR

    stopifnot(self$compartments[1,"S",] >= 0)
    stopifnot(self$compartments[1,"I",] >= 0)
    stopifnot(self$compartments[1,"R",] >= 0)
  }

  invisible(self)
})


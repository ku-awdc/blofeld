source("spread.R")

# A spillover spread module representing spread from wild boar to domestic pigs

Spillover <- R6Class("Spillover",

	inherit = Spread,

	public = list(

		setup = function(graph, beta=0.01) {

		  # Get distance as a dense matrix from igraph:
		  dist <- distances(graph)
		  # Our beta vector representing spillover from 0, 1, and 2 patches away
		  (mbetas <- c(-beta/(4^(0:2)), 0.0))
		  stopifnot(length(mbetas)==4L)
		  # Populate beta so it is a matrix of -betas based on distance:
		  private$beta <- matrix(mbetas[case_when(as.numeric(dist) > 3 ~ 3.0, TRUE ~ as.numeric(dist) + 1.0)], ncol=ncol(dist), nrow=nrow(dist))
		  stopifnot(all(!is.na(private$beta)))

		},

		update = function(statuses, control_matrix) {

		  # Spillover is density-dependent (alive infected):
		  # rv <- colSums(private$beta * statuses[,3L])
		  rv <- crossprod(private$beta, statuses[,3L])[,1]

		  return(rv)

		}

	),

	private = list(

	  beta = NULL

	),

	active = list(


	),

	lock_class = TRUE,
	cloneable = FALSE

)


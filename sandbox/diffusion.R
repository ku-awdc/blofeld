source("spread.R")

# A diffusion spread module representing slow diffusion of disease across contiguous wild boar patches

Diffusion <- R6Class("Diffusion",

	inherit = Spread,

	public = list(

		setup = function(graph, beta) {

		  # Get distance as a dense matrix from igraph:
		  private$beta <- beta
		  private$graph <- graph
		  dist <- distances(graph)
		  #dist[dist>10L] <- Inf
		  private$matrix <- 1.0 / (2^dist)
		  private$matrix <- private$matrix * (1 - diag(nrow(private$matrix)))
		  stopifnot(all(private$matrix >= 0.0), all(private$matrix <= 1.0))

		},

		update = function(statuses, control_matrix) {

		  infpres <- (1 - private$matrix * private$beta)^statuses[,3L]
		  return(1-apply(infpres,2,prod))

		}

	),

	private = list(

	  matrix = matrix(),
	  graph = NULL,
	  beta = 0.1

	),

	active = list(


	),

	lock_class = TRUE,
	cloneable = FALSE

)


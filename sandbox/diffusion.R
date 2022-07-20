source("spread.R")

# A diffusion spread module representing slow diffusion of disease across contiguous wild boar patches

Diffusion <- R6Class("Diffusion",

	inherit = Spread,

	public = list(

		setup = function(neighbours) {

		  # Create a dense matrix of border lengths in a very inefficient way:
		  N <- max(neighbours$Index)
		  mb <- max(neighbours$Border)
		  private$matrix <- matrix(0.0, nrow=N, ncol=N)
		  for(i in 1:nrow(neighbours)) private$matrix[neighbours$Index[i], neighbours$Neighbour[i]] <- neighbours$Border[i] / mb

		},

		update = function(statuses, control_matrix) {

		  infpres <- statuses[,3L] * private$matrix
		  return(1-apply(1-infpres,2,prod))

		}

	),

	private = list(

	  matrix = matrix()

	),

	active = list(


	),

	lock_class = TRUE,
	cloneable = FALSE

)


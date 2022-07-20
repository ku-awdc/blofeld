source("population.R")

library('R6')

# A very basic wild boar population representation

WildBoar <- R6Class("WildBoar",

	inherit = Population,

	public = list(

	  setup = function(patches, gamma2 = 0.5, gamma1 = 0.25) {

	    # Note: otherwise ignoring patches input for now
	    stopifnot(nrow(patches)==private$N)
	    private$patches <- matrix(0L, nrow=private$N, ncol=2)

	    private$prob_2_1 <- gamma2
	    private$prob_1_0 <- gamma1

	  },

		seed_infection = function(unit) {

		  private$patches[unit,1L] <- 2L
		  private$patches[unit,2L] <- private$time$day
		  private$change_status[unit] <- TRUE

		},

		update = function(infection_pressure, control_matrix, time_steps = 1L) {

		  self$reset_changed()

		  # We deliberately ignore control_matrix as there is no control
		  stopifnot(time_steps==1L)
		  stopifnot(length(infection_pressure)==nrow(private$patches))

		  # This is horribly inefficient:
		  go_2_1 <- as.logical(rbinom(private$N, 1, (private$patches[,1L]==2L)*private$prob_2_1))
		  go_1_0 <- as.logical(rbinom(private$N, 1, (private$patches[,1L]==1L)*private$prob_1_0))
		  go_0_2 <- as.logical(rbinom(private$N, 1, (private$patches[,1L]==0L)*infection_pressure))

		  private$patches[go_2_1,1L] <- 1L
		  private$patches[go_1_0,1L] <- 0L
		  private$patches[go_0_2,1L] <- 2L

		  newchange <- go_2_1 | go_1_0 | go_0_2
		  private$patches[newchange, 2L] <- private$time$day
		  private$change_status <- private$change_status | newchange

		}

	),

	private = list(

	  patches = matrix(),

	  prob_2_1 = 0.1,
	  prob_1_0 = 0.1

	),

	active = list(

		status = function() {
		  # c("Unit","Total","Infectious","Environment","Infected","NewInf","NewDead","NewVacc")
			rv <- matrix(0.0, nrow=private$N, ncol=8, dimnames=list(NULL, private$status_dimnames))
			rv[,1] <- 1L:private$N
			rv[,2] <- 1L  # We don't track number of wild boar here
			rv[,3] <- private$patches[,1L] / 2L
			rv[,5] <- private$patches[,1L] / 2L
			rv[,6] <- sum(private$patches[,1L]==2L & private$patches[,2L]==0L)

			return(rv)
		},

		spatial_centroid = function() {
			stop("The spatial_centroid active method must be overridden")
		},

		spatial_shape = function() {
			stop("The spatial_shape active method must be overridden")
		}

	),

	lock_class = TRUE,
	cloneable = FALSE
)

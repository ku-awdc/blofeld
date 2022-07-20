source("population.R")

library('R6')

# A very basic wild boar population representation with SIRS model

WildBoar <- R6Class("WildBoar",

	inherit = Population,

	public = list(

	  setup = function(patches, beta = 0.1, gamma = 0.25, delta = 0.0) {

	    stopifnot(nrow(patches)==private$N)
	    stopifnot(!is.null(patches$carrying_capacity))

	    private$totals <- round(patches$carrying_capacity)

	    private$compartments <- matrix(0L, nrow=private$N, ncol=3)
	    private$compartments[,1L] <- private$totals

	    private$beta <- beta
	    private$gamma <- gamma
	    private$delta <- delta

	    private$newinf <- numeric(private$N)

	  },

		seed_infection = function(unit) {

		  stopifnot(all(private$totals[unit] > 0L))

		  private$compartments[unit,2L] <- private$compartments[unit,2L] + 1L
		  private$compartments[unit,1L] <- private$compartments[unit,1L] - 1L
		  stopifnot(all(private$totals == apply(private$compartments,1,sum)))

		  private$newinf[unit] <- private$newinf[unit] + 1L

		},

		update = function(infection_pressure, control_matrix, time_steps = 1L) {

		  self$reset_changed()
		  private$newinf <- numeric(private$N)

		  # We deliberately ignore control_matrix as there is no control
		  stopifnot(time_steps==1L)
		  stopifnot(length(infection_pressure)==nrow(private$patches))
		  stopifnot(all(infection_pressure >= 0.0), all(infection_pressure <= 1.0))

		  # Total infection pressure
		  infprob <- 1 - ((1-private$beta)^private$compartments[,2] * (1-infection_pressure))

		  rec_to_sus <- rbinom(private$N, private$compartments[,3L], private$delta)
		  inf_to_rec <- rbinom(private$N, private$compartments[,2L], private$gamma)
		  sus_to_inf <- rbinom(private$N, private$compartments[,1L], infprob)

		  private$compartments[,1L] <- private$compartments[,1L] + rec_to_sus - sus_to_inf
		  private$compartments[,2L] <- private$compartments[,2L] + sus_to_inf - inf_to_rec
		  private$compartments[,3L] <- private$compartments[,3L] + inf_to_rec - rec_to_sus

		  stopifnot(all(private$totals == apply(private$compartments,1,sum)))

		  private$change_status <- private$change_status | any(sus_to_inf)>0L | any(inf_to_rec)>0L
		  private$newinf <- private$newinf + sus_to_inf

		}

	),

	private = list(

	  compartments = matrix(),
	  totals = numeric(),
	  newinf = 0L,

	  beta = 0.1,
	  gamma = 0.1,
	  delta = 0.05

	),

	active = list(

		status = function() {
		  # c("Unit","Total","Infectious","Environment","Infected","NewInf","NewDead","NewVacc")
			rv <- matrix(0.0, nrow=private$N, ncol=8, dimnames=list(NULL, private$status_dimnames))
			rv[,1] <- 1L:private$N
			rv[,2] <- private$totals - private$compartments[,3L]
			rv[,3] <- private$compartments[,2L]
			rv[,5] <- private$compartments[,2L]
			rv[,6] <- private$newinf

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

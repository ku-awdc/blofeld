source("population.R")

library('R6')

# A very basic wild boar population representation

WildBoar <- R6Class("WildBoar",

	inherit = Population,
	
	public = list(
	
		seed_infection = function(unit) {
			stop("The seed_infection public method must be overridden")
		},
		
		update = function(infection_pressure, control_matrix, time_steps = 1L) {
			stop("The update public method must be overridden")			
		}
		
	),
	
	private = list(
			
	),
	
	active = list(
		
		status = function() {
			eg <- matrix(0.0, nrow=self$N, ncol=8, dimnames=list(NULL, self$status_dinmanes))
			stop("The status active method must be overridden")
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

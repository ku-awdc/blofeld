library('R6')

# A spread module representing different routes of disease transmission between units
# This can either be within the same population or between populations or both
# Initialisation takes the following arguments:
# time - an R6 object synchronising dates etc so that seasonal effects can be included (may be ignored)
# locations - an R6 object that lazy-evaluates pairwise distances between all units (may be ignored)
# populations - a list of population objects or a single population object
# Inputs are the standard status matrix from each relevant population, as well as the control matrix
# Output is a vector of external infection pressures for each relevant unit
# TODO: distiguish between within-population and between-population spread

Spread <- R6Class("Spread",

	public = list(
	
		initialize = function(time, locations, populations, type=c("within","between","both")) {
			
			# TODO: check length of populations according to type
			
			private$time <- time
			private$locations <- locations
			private$populations <- populations
			
		},
		
		setup = function() {
			
		},
		
		update = function(statuses, control_matrix) {
			
		}
	
	),
	
	private = list(
	
		time = NULL,		
		locations = NULL,
		populations = NULL
		
	),
	
	active = list(
	
		
	),
	
	lock_class = TRUE,
	cloneable = FALSE	

)


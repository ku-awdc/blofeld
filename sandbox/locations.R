library('R6')

# A locations class that (eventually lazy-) evaluates pairwise distances/adjacencies between all units
# This is intended to be fixed for all models

Locations <- R6Class("Locations",

	public = list(
	
		initialize = function(time, populations, map = NULL) {
			
			private$time <- time
			private$populations <- populations
			private$map <- map
			
			# TODO: if NULL map then use bounding box for all coords in populations
			
		},
		
		adjacencies = function(unit, pop_from, pops_to) {
			
		},
		
		distances = function(unit, pop_from, pops_to) {
			
		},
		
		overlapping = function(unit, pop_from, pops_to) {
			
		}
	
	),
	
	private = list(
	
		time = NULL,
		
		populations = list( ),
		
		map = NULL
		
	),
	
	active = list(
		
	),
	
	lock_class = TRUE,
	cloneable = FALSE	

)


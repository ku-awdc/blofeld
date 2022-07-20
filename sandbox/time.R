library('R6')

# A time class synchronising dates etc between modules
# This is intended to be fixed for all models

Time <- R6Class("Time",

	public = list(
	
		initialize = function(start_time) {
			
			private$time <- start_time
			
		},
		
		update = function(time_steps) {
			
		}
	
	),
	
	private = list(
	
		time = 0L
		
	),
	
	active = list(
	
		get_time = function() self$time
		
	),
	
	lock_class = TRUE,
	cloneable = FALSE	

)

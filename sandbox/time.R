library('R6')

# A time class synchronising dates etc between modules
# This is intended to be fixed for all models

Time <- R6Class("Time",

	public = list(

		initialize = function(start_time) {

		  private$start_time <- start_time
		  private$internal_time <- start_time

		},

		update = function(time_steps) {

		  private$internal_time <- private$internal_time + 1L

		}

	),

	private = list(

	  start_time = 0L,
		internal_time = 0L

	),

	active = list(

		time = function() private$internal_time,
		day = function() as.numeric(private$internal_time - private$start_time, units="days")

	),

	lock_class = TRUE,
	cloneable = FALSE

)

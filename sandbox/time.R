library('R6')

# A time class synchronising dates etc between modules
# This is intended to be fixed for all models

Time <- R6Class("Time",

	public = list(

		initialize = function(start_time) {

		  stopifnot(inherits(start_time, "Date"))
		  private$start_time <- start_time
		  private$internal_time <- start_time
		  private$internal_doy <- strftime(private$internal_time, "%j") |> as.numeric()

		},

		update = function(time_steps) {

		  private$internal_time <- private$internal_time + 1L
		  private$internal_doy <- private$internal_doy + 1L
		  if(private$internal_doy > 365) private$internal_doy <- 0L
		  ## TODO: this is broken - time and doy will get out of sync on leap years

		}

	),

	private = list(

	  start_time = 0L,
		internal_time = 0L,
		internal_doy = 0L

	),

	active = list(

		time = function() private$internal_time,
		day = function() as.numeric(private$internal_time - private$start_time, units="days"),
		day_of_year = function() private$internal_doy

	),

	lock_class = TRUE,
	cloneable = FALSE

)

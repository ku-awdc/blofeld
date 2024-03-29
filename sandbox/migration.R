source("spread.R")

# A migration module representing (1) breeding within a patch and (2) the force of re-establishment of wild boar to new patches
# For simplicity both are assumed to happen exactly once per year (but at different times)
# Output is a vector of breeding/migration pressures i.e. probability of reversion to susceptibility from recovered

Migration <- R6Class("Migration",

	inherit = Spread,

	public = list(

		setup = function(graph, beta_breed=0.1, beta_migrate=0.05, doy_breed=30L, doy_migrate=50L) {

		  private$doy_breed <- doy_breed
		  private$doy_migrate <- doy_migrate

		  ## Breeding
		  private$breed <- -beta_breed

		  ## Migration
		  # Get distance as a dense matrix from igraph:
		  dist <- distances(graph)
		  # Our beta vector representing (1) within-patch migration (zero), (2-11) between-patch migration, (12) long-distance migration (zero)
		  (mbetas <- c(0.0, -beta_migrate/(2^(0:9)), 0.0))
		  stopifnot(length(mbetas)==12L)
		  # Populate migrate so it is a matrix of -betas based on distance:
		  private$migrate <- matrix(mbetas[case_when(as.numeric(dist) > 10 ~ 12.0, TRUE ~ as.numeric(dist) + 1.0)], ncol=ncol(dist), nrow=nrow(dist))
		  stopifnot(nrow(private$migrate)==ncol(private$migrate))

		},

		update = function(statuses, control_matrix) {

		  # Check the day of year to see if anything happens:
		  doy <- private$time$day_of_year
		  if(!doy %in% private$doy_breed && !doy %in% private$doy_migrate) {
		    return( numeric(nrow(statuses)) )
		  }

		  # Otherwise we do one or the other:
		  rv <- numeric(nrow(statuses))

		  # Breeding is density-dependent within patch:
		  if(doy %in% private$doy_breed) {
		    # NB: "infectious" here means alive!
		    # TODO: account for sex ratio in adults?
		    rv <- rv + (private$breed * statuses[,3L])
		  }

		  # Migration is density-dependent but we subtract the cc first:
		  if(doy %in% private$doy_migrate) {
		    # NB: "infectious" here means alive, and "total" means carrying capacity!
		    # TODO: parameter for breeding capacity ratio from cc
		    ratio <- 3
                breed_capacity <- statuses[, 2L] / ratio
                excess <- pmax(0, statuses[, 3L] - breed_capacity)
				#' excess %*% private$migrate
                # rv <- rv + colSums(private$migrate * excess)
                rv <- rv + crossprod(private$migrate, excess)[,1]
		  }

		  return(rv)

		}

	),

	private = list(

	  breed = numeric(0),
	  migrate = matrix(),
	  doy_breed = numeric(0),
	  doy_migrate = numeric(0)

	),

	active = list(


	),

	lock_class = TRUE,
	cloneable = FALSE

)


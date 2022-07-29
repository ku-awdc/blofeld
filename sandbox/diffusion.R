source("spread.R")

# A diffusion spread module representing transmission of ASF within patch plus diffusion of disease across contiguous wild boar patches

Diffusion <- R6Class("Diffusion",

	inherit = Spread,

	public = list(

		setup = function(graph, beta_freq=0.01, beta_carc=0.0001, beta_dens=0.01) {

		  ## Within-patch frequency-dependent transmission:
		  private$beta_freq <- -beta_freq

		  ## Within-patch density-dependent transmission from carcasses:
		  private$beta_carc <- -beta_carc

		  ## Migration
		  # Get distance as a dense matrix from igraph:
		  dist <- distances(graph)
		  # Our beta vector representing (1) within-patch density-dependent spread (zero), (2-11) between-patch frequency-dependent spread, (12) long-distance spread (zero)
		  (mbetas <- c(0.0, -beta_dens/(4^(0:4)), 0.0))
		  stopifnot(length(mbetas)==7L)
		  # Populate beta_dens so it is a matrix of -betas based on distance:
		  private$beta_dens <- matrix(mbetas[case_when(as.numeric(dist) > 5 ~ 7.0, TRUE ~ as.numeric(dist) + 1.0)], ncol=ncol(dist), nrow=nrow(dist))

		},

		update = function(statuses, control_matrix) {

		  # Within-patch spread is frequency-dependent (alive infected):
		  rv <- case_when(
		    statuses[,2L] == 0L ~ 0,
		    TRUE ~ private$beta_freq * statuses[,3L] / statuses[,2L]
		  )

		  # Carcass spread is within-patch and density-dependent:
		  carcasses <- statuses[,4L]
		  stopifnot(all(carcasses >= 0L))
		  rv <- rv + (carcasses * private$beta_carc)

		  # Between-patch spread is density-dependent (alive infected):
		  rv <- rv + colSums(private$beta_dens * statuses[,3L])

		  return(rv)

		}

	),

	private = list(

	  beta_freq = NULL,
	  beta_carc = NULL,
	  beta_dens = NULL

	),

	active = list(


	),

	lock_class = TRUE,
	cloneable = FALSE

)


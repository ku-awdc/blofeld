source("spread.R")

# A diffusion spread module representing slow diffusion of disease across contiguous wild boar patches

Diffusion <- R6Class("Diffusion",

	inherit = Spread,

	public = list(
	
		setup = function() {
			
		},
		
		update = function(statuses, control_matrix) {
			
		}
	
	),
	
	private = list(
	
	),
	
	active = list(
	
		
	),
	
	lock_class = TRUE,
	cloneable = FALSE	

)


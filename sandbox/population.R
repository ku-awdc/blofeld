library("R6")

# A population represents within-unit disease spread, and also possibly direct movement of animals between units
# Indirect disease transmission between units (including due to animal movements if the livestock dynamics are not modelled directly) happen elsewhere
# Units may be single-species farms, wild boar habitats, lakes, whatever - but they may not move
# All units in the same population must be the same animal type
# One farm may have multiple units if it has e.g. pigs, sheep, cattle and midges (1 unit each in 4 populations)
# Population must take inputs representing a background force of infection for each unit
# Population must produce a numeric matrix as output representing:
# UnitID - from 1 to N for this population
# TotalIndividuals - number of individuals on this unit
# InfectiousdIndividuals - number infectious (used for between-unit spread)
# InfectiousEnvironment - optional ability to increase infectiousness due to environment or carcass infectiousness (in terms of equivalent units to an infectious animal)
# InfectedIndividuals - total number of currently infected (potentially test positive) individuals
# NewInfected - total number of individuals infected since the last time step
# NewDead - total number of individuals dead (of disease) since the last time step
# NewVaccinated - total number of individuals vaccinated since the last time step
#
# TODO: annotate inputs/outputs with a vector detailing what has changed (where 0 means everything)
# TODO: populations can be either spatial point units or spatial extent units (not both)
# Note: the intention is to derive from this class


Population <- R6Class("Population",
    public = list(
        initialize = function(time, N = 1L) {
            private$time <- time
            private$N <- as.integer(N)
            private$change_status <- logical(N)
        },
        seed_infection = function(unit) {
            stop("The seed_infection public method must be overridden")
        },
        update = function(infection_pressure, control_matrix, time_steps = 1L) {
            stop("The update public method must be overridden")
        },
        reset_changed = function() {
            private$change_status[] <- FALSE
        }
    ),
    private = list(
        time = NULL,
        N = 0L,
        change_status = logical(0L),
        record_change = function(unit) {
            private$change_status[unit] <- TRUE
        },
        status_dimnames = c("Unit", "Total", "Infectious", "Environment", 
			"Infected", "NewInf", "NewDead", "NewVacc")
    ),
    active = list(
        status = function() {
            eg <- matrix(0.0, nrow = private$N, ncol = 8, 
				dimnames = list(NULL, private$status_dinmanes))
            stop("The status active method must be overridden")
        },
        changed = function() {
            private$change_status
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

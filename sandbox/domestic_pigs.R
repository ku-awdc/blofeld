source("population.R")

library("R6")

# Domestic pigs as a sentinel/sink population (spillover from wild boar)

DomesticPigs <- R6Class("DomesticPigs",
    inherit = Population,
    public = list(
        setup = function(patches) {
            stopifnot(nrow(patches) == private$N)
            stopifnot(!is.null(patches$pig_farms))

            private$farms <- patches$pig_farms
            private$log_survival <- 0
            private$log_dprob <- 0
        },
        update = function(infection_pressure, control_matrix, time_steps = 1L) {
            stopifnot(all(!is.na(infection_pressure)))
            stopifnot(length(infection_pressure) == private$N)

            self$reset_changed()

            private$log_survival <- private$log_survival + infection_pressure
            private$log_dprob <- infection_pressure
        },
        reset_inf_prob = function() {
            private$log_survival <- 0
        }
    ),
    private = list(
        farms = numeric(),
        log_survival = numeric(),
        log_dprob = numeric()
    ),
    active = list(
        status = function() {
            rv <- matrix(0.0,
                nrow = private$N, ncol = 4,
                dimnames = list(
                    NULL,
                    c("Index", "NumberOfFarms", "DailyInfProb", "CumulativeInfProb")
                )
            )
            rv[, 1] <- 1L:private$N
            rv[, 2] <- private$farms
            rv[, 3] <- 1 - exp(private$log_dprob)
            rv[, 4] <- 1 - exp(private$log_survival)

            return(rv)
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

source("population.R")

library("R6")

# A very basic wild boar population representation with SIRS model

WildBoar <- R6Class("WildBoar",
    inherit = Population,
    public = list(
        setup = function(patches, gamma = 0.25, carc_prob = 0.01, carc_gamma = 1e-3) {
            stopifnot(nrow(patches) == private$N)
            stopifnot(!is.null(patches$carrying_capacity))

            private$totals <- round(patches$carrying_capacity)

            private$compartments <- matrix(0L, nrow = private$N, ncol = 4)
            private$compartments[, 4L] <- private$totals

            private$gamma <- gamma
            private$carc_prob <- carc_prob
            private$carc_gamma <- carc_gamma

            private$newinf <- numeric(private$N)
        },
        seed_boar = function(unit) {
            private$compartments[unit, 1L] <- private$totals[unit]
            private$compartments[unit, 2L:4L] <- 0L
            stopifnot(all(private$totals == apply(private$compartments, 1, sum)))
        },
        remove_boar = function(unit) {
            private$compartments[unit, 1L:3L] <- 0L
            private$compartments[unit, 4L] <- private$totals[unit]
            stopifnot(all(private$totals == apply(private$compartments, 1, sum)))
        },
        seed_asf = function(unit, number) {
            stopifnot(all(private$totals[unit] >= number))

            private$compartments[unit, 1L:4L] <- 0L
            private$compartments[unit, 2L] <- number
            private$compartments[unit, 1L] <- private$totals[unit] - number
            stopifnot(all(private$totals == apply(private$compartments, 1, sum)))

            private$newinf[unit] <- private$newinf[unit] + number
        },
        reseed_asf = function(unit, number) {

            # If the number of infected to seed exceeds the number alive reduce it:
            number <- pmin(number, private$compartments[unit, 1L])

            private$compartments[unit, 1L] <- private$compartments[unit, 1L] - number
            private$compartments[unit, 2L] <- private$compartments[unit, 2L] + number
            stopifnot(all(private$totals == apply(private$compartments, 1, sum)))
            stopifnot(all(private$compartments >= 0L))

            private$newinf[unit] <- private$newinf[unit] + number
        },
        add_carcass = function(unit, number) {

            # If the number of carcasses to seed exceeds the number alive reduce it:
            number <- pmin(number, private$compartments[unit, 1L])

            private$compartments[unit, 1L] <- private$compartments[unit, 1L] - number
            private$compartments[unit, 3L] <- private$compartments[unit, 3L] + number
            stopifnot(all(private$totals == apply(private$compartments, 1, sum)))
            stopifnot(all(private$compartments >= 0L))

            private$newinf[unit] <- private$newinf[unit] + number
        },
        update = function(infection_pressure, migration_pressure, control_matrix, time_steps = 1L) {
            self$reset_changed()
            private$newinf <- numeric(private$N)
            private$newbirth <- numeric(private$N)
            private$newdeath <- numeric(private$N)

            stopifnot(time_steps == 1L)
            stopifnot(length(infection_pressure) == nrow(private$patches))
            stopifnot(length(migration_pressure) == nrow(private$patches))
            # We deliberately ignore control_matrix as there is no control

            infprob <- 1 - exp(infection_pressure)
            recprob <- 1 - exp(migration_pressure)

            sus_to_inf <- rbinom(private$N, private$compartments[, 1L], infprob)
            inf_to_nxt <- rbinom(private$N, private$compartments[, 2L], private$gamma)
            inf_to_ccs <- rbinom(private$N, inf_to_nxt, private$carc_prob)
            inf_to_rec <- inf_to_nxt - inf_to_ccs
            ccs_to_rec <- rbinom(private$N, private$compartments[, 3L], private$carc_gamma)
            rec_to_sus <- rbinom(private$N, private$compartments[, 4L], recprob)

            private$compartments[, 1L] <- private$compartments[, 1L] + rec_to_sus - sus_to_inf
            private$compartments[, 2L] <- private$compartments[, 2L] + sus_to_inf - inf_to_nxt
            private$compartments[, 3L] <- private$compartments[, 3L] + inf_to_ccs - ccs_to_rec
            private$compartments[, 4L] <- private$compartments[, 4L] + inf_to_rec + ccs_to_rec - rec_to_sus

            if (!all(private$totals == apply(private$compartments, 1, sum))) browser()
            stopifnot(all(private$totals == apply(private$compartments, 1, sum)))

            private$newinf <- private$newinf + sus_to_inf
            private$newbirth <- private$newbirth + rec_to_sus
            private$newdeath <- private$newdeath + inf_to_nxt
        }
    ),
    private = list(
        compartments = matrix(),
        totals = numeric(),
        newinf = 0L,
        newbirth = 0L,
        newdeath = 0L,
        gamma = numeric(),
        carc_prob = numeric(),
        carc_gamma = numeric()
    ),
    active = list(
        status_asf = function() {
            # browser()
            # c("Unit","Total","Infectious","Environment","Infected","NewInf","NewDead","NewVacc")
            rv <- matrix(0.0, nrow = private$N, ncol = 8, dimnames = list(NULL, private$status_dimnames))
            rv[, 1] <- 1L:private$N
            rv[, 2] <- private$totals - private$compartments[, 3L] - private$compartments[, 4L] # Don't count dead animals or carcasses
            rv[, 3] <- private$compartments[, 2L] # Only infected alive
            rv[, 4] <- private$compartments[, 3L] # Infected carcasses
            rv[, 5] <- private$compartments[, 2L] # Only infected alive
            rv[, 6] <- private$newinf

            return(rv)
        },
        status_pop = function() {
            # c("Unit","Total","Infectious","Environment","Infected","NewInf","NewDead","NewVacc")
            rv <- matrix(0.0, nrow = private$N, ncol = 8, dimnames = list(NULL, private$status_dimnames))
            rv[, 1] <- 1L:private$N
            rv[, 2] <- private$totals
            rv[, 3] <- private$compartments[, 1L] + private$compartments[, 2L] # Assume infected animals can breed
            rv[, 5] <- private$compartments[, 1L] + private$compartments[, 2L] # Assume infected animals can breed
            rv[, 6] <- private$newbirth

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

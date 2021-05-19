## Masks:

# Real or Int
# Farm x animal_types x disease_compartments
# Bool
# Farm x status_type
# Int
# Farm x regulator_obs
# Real
# Farm x indirect_spread


## Simple cattle notebook
farm_status <- c(Ok = 1, infected = 2, culled = 3)
animal_types <- c(calf = 1, heifer = 2, cow = 3, bull = 4)
disease_compartments <- c(S = 1, I = 2, R = 3)

ps_mask_description <- list(disease_compartments, farm_status[2])

total_herds <- 7

raw_world <-
  array(NA, dim = c(total_herds,
                    list(animal_types, disease_compartments) %>%
                      lengths() %>% sum)) %>%
  cbind(list(farm_status))

raw_world

realistic_setting <- Scenario$new() %>%
  ammend(PassiveSurveillance$new(sensitivity =.60, specificty = 0.99))

world <- World$default_world()

realistic_setting$run(world) ->
  passive_surv_only_outputs



#' #' Example of spread model, regulator, etc.
#' sketch_of_process <- function(what_components, which_farms) {
#'   #removed later
#'   q_array <- scenario.query(what_components, which_farms)
#'
#'
#'   #removed later
#' }


# PassiveSurveillance <-
#   R6::R6Class("PassiveSurveillance",
#               public =
#                 list(
#                   update = function(q_array) {
#
#                   }
#                 ))

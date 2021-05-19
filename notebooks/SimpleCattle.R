## Build World

# World contains only data and no methods

# Data:
# - Private static information:  initial states of farms including spatial coords, number of animals, etc

# - Private dynamic information:
#       - Internal states of farms, e.g. subgroups of animals within farms or potentially agent-specific information on individual animals or whatever - for simple applications this may be empty
#       - Internal states of processes e.g. regulators, spread models, etc

# - Public static information:
#       - Number of populations and farms
#       - DiseaseCompartments (fixed for all populations) and AnimalTypes (variable between populations)
#       - Spatial information e.g. pairwise distance, direction, bordering, and overlap between farms (across all populations) - may be calculated on demand

# - Public dynamic information
#       - Farm data:  Numeric Array of Farms x AnimalTypes x DiseaseCompartments
#              NB:  AnimalTypes may differ between Populations, so this must be the superset - is this inefficient?  Otherwise we could have a list of Arrays with different second dimension...
#       - Regulator data:  data frame including regulator classification for each farm, which is basically an enum for each column but where enums will differ between regulators.  May also include other static farm-level information e.g. population type and herd size if we include regulators that maintain this information as "standard"?
#       - State data:  Bool Matrix of Farms x States derived from the Authority
#             NB:  states are not mutually exclusive, e.g. a farm may be simultaneously infected and within 5km of another infected farm
#       - Infection pressure:  Numeric Matrix of Farms x IndirectSpread outputs


## OK I lied:  World has methods to Initialise and Reset
# Initialise:  Create the static information based on input population factories or whatever
# Reset:  (Re)-create the dynamic information from the internally stored static information


## Build Scenario

# Scenario contains mostly methods, but processes can contain basic static data e.g. parameters






###### OLDER STUFF BELOW HERE

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

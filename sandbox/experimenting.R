library('tidyverse')

if(Sys.info()[["user"]]=="matthewdenwood") setwd("~/Documents/GitHub/blofeld/sandbox")

# First create a single time object:
source("time.R")
dt <- as.Date("2020-01-01")
time <- Time$new(dt)

# Then create one or more population:
source("wild_boar.R")
wbpop <- WildBoar$new(time)

# Then create a single locations objct:
source("locations.R")
locations <- Locations$new(time, list(wbpop))

# Then create one or more spread modules:
source("diffusion.R")
diffusion <- Diffusion$new(time, locations, list(wbpop), type="within")

# Then create one or more reporting modules:
source("reporting.R")


# Then add a single control object:

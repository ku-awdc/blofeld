library('tidyverse')
library("sf")
library("igraph")
library("HexScape")

if(Sys.info()[["user"]]=="matthewdenwood"){
  setwd("~/Documents/GitHub/blofeld/sandbox")
}

# Create a fake patch/neighbour graph:
patches <- tibble(patch = 1L, carrying_capacity = 20L)
graph <- graph_from_data_frame(data.frame(from=numeric(), to=numeric()), vertices = patches)
distances(graph)

# First create a single time object:
source("time.R")
dt <- as.Date("2020-01-01")
time <- Time$new(dt)

# Then create one or more population:
source("wild_boar.R")
wbpop <- WildBoar$new(time, nrow(patches))
wbpop$setup(patches, gamma = 0.1, carc_prob = 0.05, carc_gamma = 1e-3)

# Then create a single locations objct:
source("locations.R")
locations <- Locations$new(time, list(wbpop))

# Spread module for migration:
source("migration.R")
migration <- Migration$new(time, locations, list(wbpop), type="within")
migration$setup(graph, beta_breed=0.1, beta_migrate=0.05, doy_breed=30L, doy_migrate=50L)

# Spread module for ASF:
source("diffusion.R")
diffusion <- Diffusion$new(time, locations, list(wbpop), type="within")
diffusion$setup(graph, beta_freq=0.25, beta_carc=0.001, beta_dens=0.01)

## Run:
wbpop$seed_boar(1L)
wbpop$seed_asf(1L, 3L)
yrs <- 1
output <- expand_grid(Year = 1:yrs, Day = 1:365, Susceptible = NA_integer_, Infected = NA_integer_, Carcass = NA_integer_, Dead = NA_integer_) |>
  arrange(Year, Day)

pb <- txtProgressBar(style=3)
for(i in 1:nrow(output)){
  output$Susceptible[i] <- wbpop$status_asf[,2L] - wbpop$status_asf[,3L]
  output$Infected[i] <- wbpop$status_asf[,3L]
  output$Carcass[i] <- wbpop$status_asf[,4L]
  output$Dead[i] <- wbpop$status_pop[,2L] - wbpop$status_pop[,3L] - wbpop$status_asf[,4L]
  stopifnot(sum(output[i,-c(1:2)]) == patches$carrying_capacity)

  beta <- diffusion$update(wbpop$status_asf)
  gamma <- migration$update(wbpop$status_pop)
  stopifnot(all(beta <= 0.0))
  stopifnot(all(gamma <= 0.0))
  wbpop$update(beta, gamma, NULL, 1)
  time$update()

  setTxtProgressBar(pb, i/nrow(output))
}
close(pb)

pltdt <- output |>
  mutate(Date = as.Date(Year*365 + Day, origin=as.Date("2000-01-01"))) |>
  pivot_longer(Susceptible:Dead, names_to="Compartment", values_to="N")

ggplot(pltdt, aes(x=Date, y=N, col=Compartment)) +
  geom_line() +
  theme_light()

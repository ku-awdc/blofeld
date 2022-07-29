library("tidyverse")
library("sf")
library("igraph")
library("HexScape")


## Switch for migration vs uniform repopulation:
USE_MIGRATION <- TRUE

## Switch for toy vs real population:
USE_TOY <- FALSE

## Patch to seed:
SEED_PATCH <- if (USE_TOY) 1L else 756L
SEED_PATCH <- if (USE_TOY) 1L else 91L # adjusted for `small_patches_sf`

## Number of years:
YEARS <- 5L


## Extremely messy code:

if (Sys.info()[["user"]] == "matthewdenwood") {
  setwd("~/Documents/GitHub/blofeld/sandbox")
  (load("/Users/matthewdenwood/Documents/Research/Papers/Hexscape paper/patches_DK032.rda"))
}

if (Sys.info()[["user"]] == "tpb398") {
  setwd("C:/Users/tpb398/Documents/GitHub/blofeld/sandbox")
  (load("C:/Users/tpb398/Documents/GitHub/blofeld_asf/rscripts/data-raw/patches_DK032.rda"))
}

# Load toy patches:
if (USE_TOY) {
  (load("toy_patches.rda"))
  patches <- patches_hole
  neighbours <- neighbours_hole
}

patches$carrying_capacity <- round(patches$carrying_capacity)
st_as_sf(patches, sf_column_name  = "geometry") ->
  patches_sf
library(tmap)
tmap_mode("view")
tmap_options(check.and.fix = TRUE)
tm_shape(patches_sf) +
  tm_polygons() +
  tm_polygons(st_as_sf(bbox_)) +
  # tm_polygons(col = "carrying_proportion") +
  # tm_fill(col = "carrying_proportion") +
  NULL


c(bottom = "DK032_0757",
  top = "DK032_1712",
  left = "DK032_0482",
  right = "DK032_2390") %>%
  enframe(value = "Index") %>%
  left_join(patches) %>% {
    sf::st_bbox(.$hex_centroid) %>%
      st_as_sfc()
  } -> bbox_
ggplot() +
  geom_sf(data = patches_sf, fill = NA) +
  geom_sf(data = bbox_, fill = NA) +
  NULL

small_patches_sf <- patches_sf[st_intersects(bbox_, patches_sf, sparse = FALSE)[1,],]

ggplot() +
  geom_sf(data = small_patches_sf, aes(fill = carrying_proportion)) +
  # geom_sf(data = bbox_) +
  NULL

patches <- patches %>% semi_join(small_patches_sf, by = "Index")
neighbours <- neighbours %>%
  semi_join(small_patches_sf, by = "Index") %>%
  semi_join(small_patches_sf, by = c("Neighbour" = "Index"))

graph <- graph_from_data_frame(neighbours, vertices = patches)

distances(graph)

# First create a single time object:
source("time.R")
dt <- as.Date("2020-01-01")
time <- Time$new(dt)

# Then create one or more population:
source("wild_boar.R")
wbpop <- WildBoar$new(time, nrow(patches))
wbpop$setup(patches, gamma = 0.1, carc_prob = 0.05, carc_gamma = 1 / 365)

source("domestic_pigs.R")
dpop <- DomesticPigs$new(time, nrow(patches))
dpop$setup(patches)
dpop$status
dpop$status %>% head()

# Then create a single locations objct:
source("locations.R")
locations <- Locations$new(time, list(wbpop))

# Spread module for migration:
source("migration.R")
migration <- Migration$new(time, locations, list(wbpop, dpop), type = "within")
doy_migrate <- 50L
migration$setup(graph, beta_breed = 0.1, beta_migrate = 0.05, doy_breed = 30L, doy_migrate = doy_migrate)

# Spread module for ASF:
source("diffusion.R")
diffusion <- Diffusion$new(time, locations, list(wbpop, dpop), type = "within")
diffusion$setup(graph, beta_freq = 0.25, beta_carc = 0.005, beta_dens = 0.001)

# Spillover module:
source("spillover.R")
spillover <- Spillover$new(time, locations, list(wbpop, dpop), type = "between")
spillover$setup(graph, beta = 0.01)

# adjusting...
# patches$Index %>% {which(. == "DK032_0756")}

## Run:
# wbpop$seed_boar(1)
wbpop$seed_boar(1:nrow(patches))
wbpop$seed_asf(SEED_PATCH, 3L)
yrs <- YEARS
output <- expand_grid(
  Year = 1:yrs, Day = 1:365,
  Spillover = NA_real_,
  Susceptible = NA_integer_, Infected = NA_integer_, Carcass = NA_integer_, Dead = NA_integer_
) |>
  arrange(Year, Day)

mkplt <- function(wbpop, dpop, year) {
  patches$popsize <- wbpop$status_pop[, "Infectious"]
  patches$infected <- factor(case_when(patches$popsize == 0L ~ "Extinct", wbpop$status_asf[, "Infectious"] == 0L ~ "No", TRUE ~ "Yes"), levels = c("Extinct", "No", "Yes"))
  patches$carcass <- factor(wbpop$status_asf[, "Environment"] > 0L, levels = c(FALSE, TRUE), labels = c("No", "Yes"))
  patches$spillover <- dpop$status[, "NumberOfFarms"] * dpop$status[, "CumulativeInfProb"]
  pt1 <- ggplot(patches, aes(geometry = geometry, fill = popsize + 1)) +
    geom_sf() +
    theme_void() +
    ggtitle(str_c("Year ", year, " - population")) +
    theme(legend.pos = "bottom") +
    scale_fill_gradient(limits = c(1, 25), trans = "log10")
  pt2 <- ggplot(patches, aes(geometry = geometry, fill = infected)) +
    geom_sf() +
    theme_void() +
    ggtitle(str_c("Year ", year, " - ASF")) +
    theme(legend.pos = "bottom") +
    scale_fill_manual(values = c(Extinct = "Grey", No = "Green", Yes = "Red"))
  pt3 <- ggplot(patches, aes(geometry = geometry, fill = carcass)) +
    geom_sf() +
    theme_void() +
    ggtitle(str_c("Year ", year, " - Carcass")) +
    theme(legend.pos = "bottom") +
    scale_fill_manual(values = c(No = "Grey", Yes = "chocolate3"))
  pt4 <- ggplot(patches, aes(geometry = geometry, fill = spillover)) +
    geom_sf() +
    theme_void() +
    ggtitle(str_c("Year ", year, " - Spillover")) +
    theme(legend.pos = "bottom")
  print(ggpubr::ggarrange(pt1, pt2, pt4, pt3, nrow = 2L, ncol = 2L))
}

pdf("output.pdf")
mkplt(wbpop, dpop, 0)
died <- FALSE
pb <- txtProgressBar(style = 3)
for (i in 1:nrow(output)) {
  output$Spillover[i] <- sum(dpop$status[, "NumberOfFarms"] * dpop$status[, "DailyInfProb"])

  output$Susceptible[i] <- sum(wbpop$status_asf[, 2L] - wbpop$status_asf[, 3L])
  output$Infected[i] <- sum(wbpop$status_asf[, 3L])
  output$Carcass[i] <- sum(wbpop$status_asf[, 4L])
  output$Dead[i] <- sum(wbpop$status_pop[, 2L] - wbpop$status_pop[, 3L] - wbpop$status_asf[, 4L])
  stopifnot(sum(output[i, -c(1:3)]) == sum(patches$carrying_capacity))

  beta <- diffusion$update(wbpop$status_asf)
  if (sum(wbpop$status_asf[, c(3, 4)]) == 0) {
    stopifnot(sum(beta) == 0)
    stopifnot(sum(wbpop$status_asf[, c(3, 4)]) == 0)
    cat("\nDied out at year - day ", str_c(output$Year[i], " - ", output$Day[i]), "\n")
    break
  }
  gamma <- migration$update(wbpop$status_pop)
  if (!USE_MIGRATION && time$day_of_year == doy_migrate) {
    gamma[] <- mean(gamma)
  }
  stopifnot(all(beta <= 0.0))
  stopifnot(all(gamma <= 0.0))
  wbpop$update(beta, gamma, NULL, 1)

  spill <- spillover$update(wbpop$status_asf)
  dpop$update(spill, NULL, 1)

  time$update()

  if (output$Day[i] %% 30 == 1L) {
    mkplt(wbpop, dpop, str_c(output$Year[i], " - ", output$Day[i]))
    dpop$reset_inf_prob()
  }

  setTxtProgressBar(pb, i / nrow(output))
}
close(pb)
dev.off()

output <- output |> mutate(Date = as.Date(Year * 365 + Day, origin = as.Date("2000-01-01")))

pltdt <- output |>
  pivot_longer(Susceptible:Dead, names_to = "Compartment", values_to = "N")

pt1 <- ggplot(pltdt, aes(x = Date, y = N, col = Compartment)) +
  geom_line() +
  theme_light()

pt2 <- ggplot(output, aes(x = Date, y = Spillover)) +
  geom_line() +
  theme_light()

print(ggpubr::ggarrange(pt1, pt2, nrow = 2, ncol = 1))
ggsave("output_summary.pdf")

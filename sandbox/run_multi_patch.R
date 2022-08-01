library("tidyverse")
library("sf")
library("tmap")
library("igraph")
library("HexScape")

## Load helper function and load real patches:
if (Sys.info()[["user"]] == "matthewdenwood") {
  setwd("~/Documents/GitHub/blofeld/sandbox")
  (load("/Users/matthewdenwood/Documents/Research/Papers/Hexscape paper/patches_DK032.rda"))
}

if (Sys.info()[["user"]] == "tpb398") {
  setwd("C:/Users/tpb398/Documents/GitHub/blofeld/sandbox")
  (load("C:/Users/tpb398/Documents/GitHub/blofeld_asf/rscripts/data-raw/patches_DK032.rda"))
}
source("multi_patch_fun.R")

## Load toy patches:
(load("toy_patches.rda"))

patches$carrying_capacity <- round(patches$carrying_capacity)
st_as_sf(patches, sf_column_name  = "geometry") ->
  patches_sf

## Restrict the real patches to south east Jutland:

bbox_ <- c(bottom = "DK032_0757",
  top = "DK032_1712",
  left = "DK032_0482",
  right = "DK032_2390") %>%
  enframe(value = "Index") %>%
  left_join(patches) %>% {
    sf::st_bbox(.$hex_centroid) %>%
      st_as_sfc()
  }

tmap_mode("view")
tmap_options(check.and.fix = TRUE)
tm_shape(patches_sf) +
  tm_polygons() +
  tm_polygons(st_as_sf(bbox_)) +
  # tm_polygons(col = "carrying_proportion") +
  # tm_fill(col = "carrying_proportion") +
  NULL

ggplot() +
  geom_sf(data = patches_sf, fill = NA) +
  geom_sf(data = bbox_, fill = NA) +
  NULL

small_patches_sf <- patches_sf[st_intersects(bbox_, patches_sf, sparse = FALSE)[1,],]

ggplot() +
  geom_sf(data = small_patches_sf, aes(fill = carrying_proportion)) +
  # geom_sf(data = bbox_) +
  NULL

small_patches <- patches %>% semi_join(small_patches_sf, by = "Index") %>% mutate(label = str_replace(Index, "DK032_", ""))
small_neighbours <- neighbours %>%
  semi_join(small_patches_sf, by = "Index") %>%
  semi_join(small_patches_sf, by = c("Neighbour" = "Index"))


ggplot(small_patches, aes(geometry=geometry, label=label, fill=carrying_capacity)) + geom_sf() + geom_sf_text() + coord_sf(xlim=c(4270000, 4280000), ylim=c(3520000, 3530000))
seed_patches <- which(small_patches$Index %in% str_c("DK032_0", c(687,688,755,756,821,822)))


## Options for running:
# results <- multi_patch_fun(iteration=1L, patches=patches, neighbours=neighbours, use_migration=TRUE, seed_patch=756L, years=5L, plot=FALSE)
# results <- multi_patch_fun(iteration=1L, patches=small_patches, neighbours=small_neighbours, use_migration=TRUE, seed_patch=seed_patches, years=10L, plot=TRUE, progbar=TRUE)
# results <- multi_patch_fun(iteration=1L, patches=patches_hole, neighbours=neighbours_hole, use_migration=TRUE, seed_patch=1L, years=5L, plot=FALSE)

multi_patch_fun(
  use_migration = TRUE,
  iteration = 1L,
  patches = small_patches,
  neighbours = small_neighbours,
  seed_patch = seed_patches,
  years = 5L,
  plot = TRUE,
  progbar = TRUE,
  plot_name = "with_diffusion_migration"
)

multi_patch_fun(
  use_migration = FALSE,
  iteration = 1L,
  patches = small_patches,
  neighbours = small_neighbours,
  seed_patch = seed_patches,
  years = 5L,
  plot = TRUE,
  progbar = TRUE,
  plot_name = "with_constant_migration"
)

stop()

## Full simulation
iters <- 100
years <- 5L

# Note:  shared memory forking is a problem with reference classes!
# PSOCK should work but need to export the right things, and maybe enable cloning in the reference classes?
cl <- parallel::makePSOCKcluster(max(getOption("mc.cores", 2L), 10L))
cl <- NULL

results_modelA <-
  pbapply::pblapply(
    1:iters,
    multi_patch_fun,
    cl = cl,
    patches = small_patches,
    neighbours = small_neighbours,
    use_migration = FALSE,
    seed_patch = seed_patches,
    years = years,
    plot = FALSE,
    progbar = FALSE
  ) |> bind_rows() |> mutate(Model = "A")

results_modelB <-
  pbapply::pblapply(
    1:iters,
    multi_patch_fun,
    cl = cl,
    patches = small_patches,
    neighbours = small_neighbours,
    use_migration = TRUE,
    seed_patch = seed_patches,
    years = years,
    plot = FALSE,
    progbar = FALSE
  ) |> bind_rows() |> mutate(Model = "B")

parallel::stopCluster(cl)

output <- bind_rows(results_modelA, results_modelB)


## Then make some plots to compare the models:

pltdt <- output |>
  # pivot_longer(Susceptible:Dead, names_to = "Compartment", values_to = "N") |>
  mutate(TotalAlive = Susceptible + Infected) |>
  pivot_longer(TotalAlive, names_to = "Compartment", values_to = "N") |>
  group_by(Date, Model, Compartment) |>
  summarise(Mean = mean(N), LCI = quantile(N, 0.025), UCI = quantile(N, 0.975), .groups="drop")

pt1 <- ggplot(pltdt, aes(x = Date, y = Mean, col = Model, fill=Model, ymin=LCI, ymax=UCI)) +
  geom_ribbon(alpha=0.2, col="transparent") +
  geom_line() +
  theme_light() +
  # facet_wrap(~ Model, ncol = 1) +
  NULL
pt1
spillover <- output |>
  group_by(Date, Model) |>
  summarise(Mean = mean(Spillover), LCI = quantile(Spillover, 0.025), UCI = quantile(Spillover, 0.975), .groups="drop")

pt2 <- ggplot(spillover, aes(x = Date, y = Mean, col=Model, fill=Model, ymin=LCI, ymax=UCI)) +
  # geom_ribbon(alpha=0.2, col="transparent") +
  geom_line() +
  # facet_wrap(~Model, ncol = 1) +
  theme_light()

print(ggpubr::ggarrange(pt1, pt2, nrow = 2, ncol = 1))
ggsave("output_presentation.png")


#'---
#'---
#'

#+ setup, message=FALSE,warning=FALSE
library(magrittr)
library(tidyverse)
library(igraph)
library(ggfortify)
#'
#' Use the FMD model-data to make up plausible herd-sizes.
#'
population_df <-
  read_delim(r"(C:\Users\tpb398\Documents\GitHub\DTU-DADS3.1-New Data\DataDADS.csv)",
             delim = ";", locale = readr::locale("da"))
herdtypes <- read_csv2("../DTU-DADS3.1-New Data/typesfile.csv")
# herdtypes
#'
#'
population_df <- population_df %>%
  left_join(herdtypes %>% select(herdTypeID, HerdTypeName),
            by = c("herdType" = "herdTypeID"))
population_df %>%
  glimpse()

glm(herdSize ~ HerdTypeName - 1, data = population_df %>% filter(cows!=0),
    family = poisson) %>%
  broom::tidy()

#'
#'
cattle_herd_size_distr <- \(n) rpois(n = n, lambda = exp(c(3.42, 5.90, 4.91)[sample.int(n = 3, size = n, replace = TRUE)]))

#' Make a good amount of herds, and connect them through a ring-graph.
#' Then for immediate neighbours, make a 10% likeliness to infect.
#'
total_herds <- 140
#'
#'
herds <- make_ring(total_herds, circular = TRUE)
print_all(herds)
herds %>%
  plot()
#'
#'
herds %>%
  igraph::as_data_frame() %>%
  as_tibble() %>%
  mutate(infection_probability = qlogis(.10))
#'
V(herds)$farm_id <- V(herds) %>% as_ids()
V(herds)$herd_size <- cattle_herd_size_distr(total_herds)
#'
output_dir <- "../epi_bevy/assets/ring_population";
usethis::use_directory(output_dir %>% base::dirname());
#'
#'
herds %>%
  igraph::as_data_frame("vertices") %T>%
  jsonlite::write_json(x = ., path = file.path(dirname(output_dir), "population_info.json")) %>%
  # what does this actually look like
  jsonlite::toJSON() %>%
  jsonlite::prettify()

#'
herds %>%
  neighborhood(order = 1) %>%
  enframe("farm_id", "adjacent") %>%
  mutate(
    # assume that the first one is itself.
    adjacent = adjacent %>% map(. %>% `[`(-1)),
    adjacent = adjacent %>% map(. %>% igraph::as_ids() %>% as.integer)) %>% {
    #TODO remove the filename from the path -- what package does this?
    #TODO put this somewhere else
    jsonlite::write_json(x = ., path = file.path(dirname(output_dir), "ring_adjacency.json"));
    .
  } %>%
  # what does this actually look like
  jsonlite::toJSON() %>%
  jsonlite::prettify()

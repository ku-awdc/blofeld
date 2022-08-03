library("tidyverse")
library("sf")
library("tmap")
library("igraph")
library("HexScape")


## TODO: clean up the framework and incorporate into the Blofeld package
source("time.R")
source("wild_boar.R")
source("domestic_pigs.R")
source("locations.R")
source("migration.R")
source("diffusion.R")
source("spillover.R")


## Code is now slightly tidied up into a function
multi_patch_fun <-
  function(iteration,
           patches,
           neighbours,
           use_migration = TRUE,
           seed_patch = 1L,
           years = 5L,
           plot = FALSE,
           progbar = FALSE,
           plot_name = "output") {

    ## Switch for migration vs uniform repopulation:
    USE_MIGRATION <- use_migration

    ## Some other things that probably should be parameters:
    doy_migrate <- 50L
    doy_breed <- 30L
    doy_incursion <- 60L

    ## Note: seed_patch can be a vector,
    # in which case a different one will be chosen each year

    graph <- graph_from_data_frame(neighbours, vertices = patches)
    # distances(graph)

    # First create a single time object:
    dt <- as.Date("2020-01-01")
    time <- Time$new(dt)

    # Then create one or more population:
    wbpop <- WildBoar$new(time, nrow(patches))
    wbpop$setup(patches, gamma = 0.1, carc_prob = 0.1, carc_gamma = 1 / 30)

    dpop <- DomesticPigs$new(time, nrow(patches))
    dpop$setup(patches)
    dpop$status
    dpop$status %>% head()

    # Then create a single locations objct:
    locations <- Locations$new(time, list(wbpop))

    # Spread module for migration:
    migration <- Migration$new(time, locations, list(wbpop, dpop), type = "within")
    migration$setup(graph,
      beta_breed = 0.1, beta_migrate = 0.025,
      doy_breed = doy_breed, doy_migrate = doy_migrate
    )

    # Spread module for ASF:
    diffusion <- Diffusion$new(time, locations, list(wbpop, dpop), type = "within")
    diffusion$setup(graph, beta_freq = 0.2, beta_carc = 0.0025, beta_dens = 0.0015)

    # Spillover module:
    spillover <- Spillover$new(time, locations, list(wbpop, dpop), type = "between")
    spillover$setup(graph, beta = 0.002)

    ## Run:
    # wbpop$seed_boar(1)
    wbpop$seed_boar(1:nrow(patches))
    output <- expand_grid(
      Iteration = iteration,
      Year = 1:years, Day = 1:365,
      Spillover = NA_real_,
      Susceptible = NA_integer_, Infected = NA_integer_, Carcass = NA_integer_, Dead = NA_integer_
    ) |>
      arrange(Year, Day)

    mkplt <- function(wbpop, dpop, year) {  
      patches$popsize <- wbpop$status_pop[, "Infectious"]
      patches$infected <- factor(case_when(patches$popsize == 0L ~ "Extinct", 
        wbpop$status_asf[, "Infectious"] == 0L ~ "No", TRUE ~ "Yes"), 
        levels = c("Extinct", "No", "Yes"))
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
      # pt3 <- ggplot(patches, aes(geometry = geometry, fill = carcass)) +
      #   geom_sf() +
      #   theme_void() +
      #   ggtitle(str_c("Year ", year, " - Carcass")) +
      #   theme(legend.pos = "bottom") +
      #   scale_fill_manual(values = c(No = "Grey", Yes = "chocolate3"))
      # pt4 <- ggplot(patches, aes(geometry = geometry, fill = spillover)) +
      #   geom_sf() +
      #   theme_void() +
      #   ggtitle(str_c("Year ", year, " - Spillover")) +
      #   theme(legend.pos = "bottom")
      # print(ggpubr::ggarrange(pt1, pt2, pt4, pt3, nrow = 2L, ncol = 2L))
      print(ggpubr::ggarrange(pt1, pt2, nrow = 1, ncol = 2))
    }

    if (plot) {
      pdf(str_c(plot_name, ".pdf"), paper = "a4r")
      mkplt(wbpop, dpop, 0)
    }

    if (progbar) pb <- txtProgressBar(style = 3)
    for (i in 1:nrow(output)) {
      output$Spillover[i] <- sum(dpop$status[, "NumberOfFarms"] * dpop$status[, "DailyInfProb"])

      output$Susceptible[i] <- sum(wbpop$status_asf[, 2L] - wbpop$status_asf[, 3L])
      output$Infected[i] <- sum(wbpop$status_asf[, 3L])
      output$Carcass[i] <- sum(wbpop$status_asf[, 4L])
      output$Dead[i] <- sum(wbpop$status_pop[, 2L] - wbpop$status_pop[, 3L] - wbpop$status_asf[, 4L])
      stopifnot(sum(output[i, -c(1:4)]) == sum(patches$carrying_capacity))

      # Introduce ASF on the specified day of year:
      if (time$day_of_year == doy_incursion) {
        sp <- seed_patch[sample.int(length(seed_patch), 1L)]
        wbpop$add_carcass(sp, 1L)
        wbpop$reseed_asf(sp, 2L)
      }

      beta <- diffusion$update(wbpop$status_asf)
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
        if (plot) mkplt(wbpop, dpop, str_c(output$Year[i], " - ", output$Day[i]))
        dpop$reset_inf_prob()
      }

      if (progbar) setTxtProgressBar(pb, i / nrow(output))
    }
    if (progbar) close(pb)
    if (plot) dev.off()

    output <- output |> mutate(Date = as.Date(Year * 365 + Day, origin = as.Date("2000-01-01")))

    # if(plot) {
    #   pltdt <- output |>
    #     pivot_longer(Susceptible:Dead, names_to = "Compartment", values_to = "N")
    #
    #   pt1 <- ggplot(pltdt, aes(x = Date, y = N, col = Compartment)) +
    #     geom_line() +
    #     theme_light()
    #
    #   pt2 <- ggplot(output, aes(x = Date, y = Spillover)) +
    #     geom_line() +
    #     theme_light()
    #
    #   print(ggpubr::ggarrange(pt1, pt2, nrow = 2, ncol = 1))
    #   ggsave("output_summary.pdf")
    # }

    return(output)
  }

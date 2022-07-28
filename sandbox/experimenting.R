library('tidyverse')
library("sf")
library("igraph")

if(Sys.info()[["user"]]=="matthewdenwood") setwd("~/Documents/GitHub/blofeld/sandbox")

# Load the toy patches (these are simply copied from HexScape):
(load("toy_patches.rda"))
patches <- patches_hole
neighbours <- neighbours_hole

(load("/Users/matthewdenwood/Documents/Research/Papers/Hexscape paper/patches_DK032.rda"))

ptl <- patches$Index
patches$Index <- as.numeric(factor(patches$Index, levels=ptl))
neighbours$Index <- as.numeric(factor(neighbours$Index, levels=ptl))
neighbours$Neighbour <- as.numeric(factor(neighbours$Neighbour, levels=ptl))

graph <- graph_from_data_frame(neighbours, vertices=patches)

ggplot(patches, aes(geometry=geometry)) + geom_sf() + theme_void()


# First create a single time object:
source("time.R")
dt <- as.Date("2020-01-01")
time <- Time$new(dt)

# Then create one or more population:
source("wild_boar.R")
wbpop <- WildBoar$new(time, nrow(patches))
wbpop$setup(patches, gamma=0.01)

# Then create a single locations objct:
source("locations.R")
locations <- Locations$new(time, list(wbpop))

# Spread module for migration:
source("migration.R")
migration <- Migration$new(time, locations, list(wbpop), type="within")
migration$setup(graph, 0.1, 0.05, 30, 50)

# Spread module for ASF:
source("diffusion.R")
diffusion <- Diffusion$new(time, locations, list(wbpop), type="within")
diffusion$setup(graph, 0.001, 0.01)

mkplt <- function(wbpop, year){
  patches$popsize <- wbpop$status_pop[,"Infectious"]
  patches$infected <- case_when(patches$popsize == 0L ~ NA_real_, TRUE ~ pmin(1L, wbpop$status_asf[,"Infectious"]))
  pt1 <- ggplot(patches, aes(geometry=geometry, fill=popsize+1)) + geom_sf() + theme_void() + ggtitle(str_c("Year ", year, " - population")) + theme(legend.pos="bottom") + scale_fill_gradient(limits=c(1,25), trans="log10")
  pt2 <- ggplot(patches, aes(geometry=geometry, fill=infected)) + geom_sf() + theme_void() + ggtitle(str_c("Year ", year, " - ASF")) + theme(legend.pos="bottom") + scale_fill_gradient(limits=c(0,1), high="#CB4C4E")
  print(ggpubr::ggarrange(pt1, pt2))
}

## Run:
wbpop$seed_boar(756)
yrs <- 4

pdf("output.pdf")
mkplt(wbpop, 0)
for(i in 1:yrs){
  if(i == 5){
    wbpop$seed_asf(756)
  }
  pbapply::pbsapply(1:365, function(x){
    if(i < 5){
      beta <- numeric(nrow(patches))
    }else{
      beta <- diffusion$update(wbpop$status_asf)
    }
    gamma <- migration$update(wbpop$status_pop)
    stopifnot(all(beta <= 0.0))
    stopifnot(all(gamma <= 0.0))
    #if(sum(beta+gamma) != 0) browser()
    wbpop$update(beta, gamma, NULL, 1)
    time$update()
  })
  mkplt(wbpop, i)
  print(i)
}
dev.off()


pdf("output.pdf")
wbpop$seed_asf(756)
mkplt(wbpop, 0)
time$.__enclos_env__$private$internal_doy <- 0
for(i in 1:20){
  beta <- diffusion$update(wbpop$status_asf)
  gamma <- migration$update(wbpop$status_pop)
  stopifnot(all(beta <= 0.0))
  stopifnot(all(gamma <= 0.0))
  #if(sum(beta+gamma) != 0) browser()
  wbpop$update(beta, gamma, NULL, 1)
  time$update()
  mkplt(wbpop, time$day_of_year)
  print(i)
}
dev.off()


stop()


sum(wbpop$status_asf[,"Total"])
sum(wbpop$status_pop[,"Infectious"])
beta <- numeric(nrow(patches))
sum(beta)
gamma <- migration$update(wbpop$status_pop)
sum(gamma)
wbpop$update(beta, gamma, NULL, 1)
time$update()
time$day_of_year




wbpop$seed_infection(which(patches$carrying_capacity > 10)[1])
inflev <- c("Infected","Uninfected","Removed")

pdf("output.pdf")
patches$status <- factor(case_when(wbpop$status[,3]>0L~"Infected", wbpop$status[,2]>0L~"Uninfected", TRUE~"Removed"), levels=inflev)
ggplot(patches, aes(geometry=geometry, fill=status)) + geom_sf() + scale_fill_discrete(drop=FALSE) + theme_void()
for(i in 1:20){
  wbpop$update(diffusion$update(wbpop$status))
  wbpop$status
  patches$status <- factor(case_when(wbpop$status[,3]>0L~"Infected", wbpop$status[,2]>0L~"Uninfected", TRUE~"Removed"), levels=inflev)
  print(ggplot(patches, aes(geometry=geometry, fill=status)) + geom_sf() + scale_fill_discrete(drop=FALSE) + theme_void())
  print(i)
}
dev.off()

# Then create one or more reporting modules:
# source("reporting.R")


# Then add a single control object:

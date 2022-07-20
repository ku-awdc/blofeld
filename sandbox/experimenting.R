library('tidyverse')
library("sf")

if(Sys.info()[["user"]]=="matthewdenwood") setwd("~/Documents/GitHub/blofeld/sandbox")

# Load the toy patches (these are simply copied from HexScape):
(load("toy_patches.rda"))

patches <- patches_hole
neighbours <- neighbours_hole

ptl <- patches$Index
patches$Index <- as.numeric(factor(patches$Index, levels=ptl))
neighbours$Index <- as.numeric(factor(neighbours$Index, levels=ptl))
neighbours$Neighbour <- as.numeric(factor(neighbours$Neighbour, levels=ptl))

ggplot(patches, aes(geometry=geometry)) + geom_sf() + theme_void()


# First create a single time object:
source("time.R")
dt <- as.Date("2020-01-01")
time <- Time$new(dt)

# Then create one or more population:
source("wild_boar.R")
wbpop <- WildBoar$new(time, nrow(patches))

# Then create a single locations objct:
source("locations.R")
locations <- Locations$new(time, list(wbpop))

# Then create one or more spread modules:
source("diffusion.R")
diffusion <- Diffusion$new(time, locations, list(wbpop), type="within")
diffusion$setup(neighbours)


wbpop$setup(patches)
wbpop$seed_infection(1L)

inflev <- c("Uninfected","Chronic","Acute")

pdf("output.pdf")
patches$status <- factor(inflev[(wbpop$status[,3]*2)+1L], levels=inflev)
ggplot(patches, aes(geometry=geometry, fill=status)) + geom_sf() + scale_fill_discrete(drop=FALSE)
for(i in 1:20){
  wbpop$update(diffusion$update(wbpop$status))
  wbpop$status
  patches$status <- factor(inflev[(wbpop$status[,3]*2)+1L], levels=inflev)
  print(ggplot(patches, aes(geometry=geometry, fill=status)) + geom_sf() + scale_fill_discrete(drop=FALSE))
}
dev.off()

# Then create one or more reporting modules:
source("reporting.R")


# Then add a single control object:

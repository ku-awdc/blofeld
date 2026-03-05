## Quick & dirty multi-group model:  instructions for Sandra

# If necessary:
# install.packages("blofeld", repos=c(CRAN="https://cran.rstudio.com/", "ku-awdc"="https://ku-awdc.github.io/drat/"))
# Hopefully this compiles/installs without problems but let me know if not!

# Just to check:
stopifnot(packageVersion("blofeld") >= "0.2.1")

# Other needed packages:
library("tidyverse")
library("Rcpp")

# Compile:
sourceCpp("quick_multi_group.cpp")
# NOTE: adjust numE/numI/numR as required in the .cpp file

## Either use the deterministic:
Group <- DeterministicGroup
Pop <- DeterministicPop
## Or stochastic version:
Group <- StochasticGroup
Pop <- StochasticPop
## Note: both are compiled together; no need to have two separate cpp files!

## Set within-group parameters as usual e.g.:
pars <- list(beta_subclin = 0, beta_clinical = 0, d_time = 1/(24*60))

## Set up a list of e.g. 2000 groups each with 2 susceptible:
G <- 2000
gps <- lapply(seq_len(G), \(x){
  gp <- new(Group)
  gp$set_parameters(pars)
  gp$set_state(list(S = 2), distribute=TRUE)
  gp
})

## Then convert into a population:
pop <- new(Pop, gps)
## Note: after this point, you MUST ensure that pop is kept alive for at least as long as gps - otherwise you will get a segfault if you access a group

## This shows the total number of animals in each compartment:
pop$getState()

## Set a beta matrix (must have G rows and columns):
bm <- 0.0001 * (1-diag(G))
pop$setBetaMatrix(bm)
## Note: this transmission is all density-dependent, and non-zero diagonals are allowed (this represents additional within-group density-dependent transmission) - to avoid confusion you could keep the diagonals always zero, as I have done here

## Now you can manipulate the groups via the list e.g.:
gps[[1]]$set_state(list(I = 1, S = 1), distribute=TRUE)
## And the results will be reflected in the population:
pop$getState()

## To run the population for e.g. 1 day (assuming d_time is 1 min):
pop$update(24*60)
## This also returns the current state, just like the update method for groups

## If you want details at group level at any point:
lapply(gps, \(x) x$get_state()) |> bind_rows() |> rowid_to_column("Group")

## Note: you can modify the beta matrix and/or groups as needed during the simulation

## It is important to release the groups before the population:
rm(gps); gc()
rm(pop); gc()

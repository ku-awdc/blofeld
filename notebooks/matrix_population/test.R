library("tidyverse")
library("Rcpp")

rm(list=ls()); gc(); sourceCpp("notebooks/matrix_population/test.cpp")

## Either:
Group <- DeterministicGroup
Pop <- DeterministicPop
## Or:
Group <- StochasticGroup
Pop <- StochasticPop

## Within-group parameters as usual e.g.:
pars <- list(beta_subclin = 0, beta_clinical = 0, reversion = 0, d_time = 1/(24*60))

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
bm <- 0.01 * (1-diag(G))
bm[,c(2,3)] <- 0
pop$setBetaMatrix(bm)

## Now you can manipulate the groups via the list e.g.:
gps[[1]]$set_state(list(I = 1, S = 1), distribute=TRUE)
## And the results will be reflected in the population:
pop$getState()

## To run the population for e.g. 1 day (assuming d_time is 1 min):
system.time(as.list(1) |> lapply(\(x) pop$update(24*60/15, 15)))
pop$getState()
system.time(as.list(1:24) |> lapply(\(x) pop$update(60/15, 15)))
pop$getState()

## If you want details at group level at any point:
lapply(gps, \(x) x$get_state()) |> bind_rows() |> rowid_to_column("Group")

## Note: you can modify the beta matrix and/or groups as needed during the simulation

## It is important to release the groups before the population:
rm(gps); gc()
rm(pop); gc()


# Very rough testing:
tts <- lapply(seq_len(10), \(x){
  gp <- new(SIRGroup)
  gp$set_parameters(list(beta_subclin = 0, beta_clinical = 0, d_time = 1/(24*60)))
  gp$set_state(list(S = 2), distribute=TRUE)
  gp
})
tts[[1]]$set_state(list(I = 1, S = 1), distribute=TRUE)
mg <- new(MPop, tts)
mg$getState()
mg$update(24*60)
mm <- matrix(0, nrow=length(tts), ncol=length(tts))
mm <- diag(length(tts))
mm[1,] <- 0.1
mg$setBetaMatrix(mm)
mg$update(24*60, 1)
lapply(tts, \(x) x$get_state()) |> bind_rows()

tts <- lapply(seq_len(2000), \(x){
  gp <- new(SIRGroup)
  gp$set_state(list(S = 2), distribute=TRUE)
  gp
})
tts[[1]]$set_state(list(I = 1, S = 1), distribute=TRUE)
mg <- new(MPop, tts)
mg$getState()
mg$setBetaMatrix(matrix(0.01, nrow=length(tts), ncol=length(tts)))

system.time(mg$update(24*60, 1))

mg$getState()
lapply(tts, \(x) x$get_state()) |> bind_rows()
mg$update(1)
tts[[1]]$get_full_state()
tts[[1]]$set_state(list(I = c(1,1,1)))



mg
tt
tt$get_state()
t2 <- mg$getGroup(0)
t2$update(10)
t2$get_state()

mg$getGroup(0)$update(7)

mg$getGroup(0)$get_state()
mg$getGroup(1)$get_state()

## Note: t2 MUST be deleted before mg

t2
t2$add(0.25)
t2
mg

mg$test()
mg
tt
t2
rm(t2); gc()
rm(mg); gc()
rm(tts); gc()
rm(tt); gc()

(t3 <- mg$copyGroups())
t3

mg2 <- new(MPop, t3)
mg2

obj <- ls()
obj <- obj[!obj%in%c("MPop","Test")]
rm(list=obj); gc()


c(0,1) |>
  as.list() |>
  map(\(p){
    gp <- Group$new()
    gp$get_state()

    gp$get_parameters()
    d_time <- 1/(24*60)
    gp$set_parameters(list(beta_clinical = 2, incubation=1, recovery=0, death=0, mortality_E=0, mortality_I=0, contact_power=p, d_time=d_time))

    gp$set_state(list(S=99, E=1), TRUE)
    gp$get_state()

    1:24 |>
      as.list() |>
      map(\(x){
        gp$update(60)
      }) |>
      bind_rows() |>
      pivot_longer(-Time) |>
      mutate(Power = p)
  }) |>
  bind_rows() |>
  ggplot(aes(x=Time, y=value, col=name)) +
  geom_line() +
  facet_wrap(~Power)


gp$get_full_state()
gp$get_state()
gp$set_state(list(S=1, E=1), TRUE)
gp$get_full_state()$E
gp$get_state()
gp$external_infection <- 0

gp$update(1/d_time)
gp$get_state()



library("tidyverse")
library("Rcpp")

rm(list=ls()); gc(); sourceCpp("notebooks/matrix_population/test.cpp")

tts <- lapply(seq_len(2000), \(x){
  gp <- new(SIRGroup)
  gp$set_state(list(S = 2), distribute=TRUE)
  gp
})
tts[[1]]$set_state(list(I = 1, S = 1), distribute=TRUE)
mg <- new(MPop, tts)
mg$getState()
mg$setBetaMatrix(matrix(0.01, nrow=length(tts), ncol=length(tts)))

system.time(mg$update(24*60))

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



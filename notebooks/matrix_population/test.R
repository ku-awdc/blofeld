library("tidyverse")
library("Rcpp")

sourceCpp("notebooks/matrix_population/test.cpp")

tt <- new(Test)
tt

tts <- list(tt, tt)
names(tts) <- c("A","B")

mg <- new(MPop, tts)
mg
tt

t2 <- mg$getGroup(0)
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



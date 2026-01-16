library("Rcpp")

# install.packages('blofeld', repos=c(CRAN="https://cran.rstudio.com/", "ku-awdc"="https://ku-awdc.github.io/drat/"))
try(rm(gp)); gc(); sourceCpp("notebooks/testRcpp.cpp")

gp <- Group$new()
gp$get_state()

gp$get_parameters()
d_time <- 1/1000
gp$set_parameters(list(incubation=0.1, recovery=0, death=0, mortality_E=0, mortality_I=0, d_time=d_time))

gp$get_full_state()
gp$get_state()
gp$set_state(list(S=1, E=1), TRUE)
gp$get_full_state()$E
gp$get_state()
gp$external_infection <- 0

gp$update(1/d_time)
gp$get_state()



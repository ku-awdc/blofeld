library("Rcpp")

try(rm(gp)); gc(); sourceCpp("notebooks/testRcpp.cpp")

gp <- Group$new()
gp$get_state()

gp <- GroupWrapper$new()
gp$get_parameters()
gp$set_parameters(list(d_time=0.01))

gp$get_full_state()
gp$get_state()
gp$set_state(list(S=10.0, E=0.0), TRUE)
gp$get_full_state()$E
gp$get_state()
gp$external_infection <- 0

gp$update(1000)
gp$get_state()



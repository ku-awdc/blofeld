library("Rcpp")

try(rm(gp)); gc(); sourceCpp("inst/include/testRcpp.cpp")
test()

gp <- GroupWrapper$new()
gp$get_parameters()
gp$set_parameters(list(d_time=0.01))

gp$get_full_state()
gp$get_state()
gp$set_state(list(S=10.0, E=1.0), TRUE)
gp$get_full_state()$E
gp$get_state()

gp$update(10000)
gp$get_state()



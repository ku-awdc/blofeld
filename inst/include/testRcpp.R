library("Rcpp")

sourceCpp("inst/include/testRcpp.cpp")
test()

gp <- GroupWrapper$new()
gp$get_parameters()
gp$get_full_state()
gp$get_state()
gp$set_state(list(S=1.0, E=1.0, D=1.0), TRUE)
gp$get_full_state()$E
gp$get_state()

gp$update(1000)
gp$get_state()



library("Rcpp")

sourceCpp("inst/include/testRcpp.cpp")
test()

gp <- GroupWrapper$new()
gp$get_parameters()
gp$get_full_state()

gp$update(10)

Rcpp::sourceCpp("notebooks/matrix_algebra/eigen_vs_naive.cpp")

dd <- 2500
mm <- matrix(rnorm(dd^2) |> abs(), ncol=dd)
tm <- t(mm)
vv <- rnorm(dd) |> abs()

## Naive R:
system.time(nr <- sapply(1:dd, \(x) sum(vv*mm[,x])))

## Matrix mult in R:
vd <- matrix(0,ncol=dd,nrow=dd)
diag(vd) <- vv
system.time(mr <- rowSums(tm %*% vd))

## Naive Rcpp:
system.time(nc <- getinf_naive(mm, vv))

## Optimised Rcpp:
system.time(oc <- getinf_opt(tm, vv))

## Using Eigen asDiagonal and .rowwise.sum()
system.time(ec <- getinf_eigen(tm, vv))

# nr; mr; nc; oc; ec

# If you want multiply column 0 of a matrix mat with v(0), column 1 with v(1), and so on, then use mat = mat * v.asDiagonal().

max(abs(nr-mr))
max(abs(nr-nc))
max(abs(nr-ec))
max(abs(nc-ec))

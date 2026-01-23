library("blofeld")
library("IPDMR")

cpts <- 1
I <- rep(5/cpts, cpts)
gamma <- 0.1
mort <- 0.1
d_time <- 0.1

cc <- new(blofeld:::Comp)
stopifnot(length(cc$values)==cpts)

getr <- \(g,m){
  dd <- apply_rates(I, c(g*cpts,m), d_time, "deterministic")
  c(dd[nrow(dd),1], sum(dd[,2]))
}
getc <- \(g,m){
  cc <- new(blofeld:::Comp)
  cc$sum <- sum(I)
  cc$process(g*d_time, m*d_time) |> unlist()
}
getr(0,0); getc(0,0)
getr(gamma,0); getc(gamma,0)
getr(0,mort); getc(0,mort)
getr(gamma,mort); getc(gamma,mort)


## TODO: competing rates means death takes a back seat - is that intended?


apply_rates(I, c(0,0), d_time, "deterministic") |> apply(2,sum)
apply_rates(I, c(gamma*cpts,0), d_time, "deterministic") |> apply(2,sum)
apply_rates(I, c(0,mort), d_time, "deterministic") |> apply(2,sum)
apply_rates(I, c(gamma*cpts,mort), d_time, "deterministic") |> apply(2,sum)
I * (1 - exp(-gamma*d_time))

cc <- new(blofeld:::Comp); cc$sum <- sum(I); cc$process(0, 0)
cc <- new(blofeld:::Comp); cc$sum <- sum(I); cc$process(gamma, 0)
cc <- new(blofeld:::Comp); cc$sum <- sum(I); cc$process(0, mort)
cc <- new(blofeld:::Comp); cc$sum <- sum(I); cc$process(gamma, mort)



cc$insert(carry$Carry)
take <- take + carry$Take
cc$update()

cc$sum
take+cc$sum

carry$Take
1-exp(-0.1)

carry$Carry
1-exp(-0.1)

IPDMR::apply_rates

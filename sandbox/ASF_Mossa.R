## Code for Mossa on ASF spread

library('tidyverse')
library("sf")
library("igraph")

(load("/Users/matthewdenwood/Documents/Research/Papers/Hexscape paper/patches_DK032.rda"))

## Ensure that patch numbering is consistent:
ptl <- patches$Index
patches$Index <- as.numeric(factor(patches$Index, levels=ptl))
neighbours$Index <- as.numeric(factor(neighbours$Index, levels=ptl))
neighbours$Neighbour <- as.numeric(factor(neighbours$Neighbour, levels=ptl))

## Setup steps to create dense matrix (could be done in R and passed to Rust?):
graph <- graph_from_data_frame(neighbours, vertices=patches)
dist <- distances(graph)
# This is a matrix of distances from 0 to Inf:
dist[1:5,1:5]
# Our beta vector representing (1) within-patch spread, (2-11) between-patch spread, (12) zero spread
(betas <- c(0.01, 5e-4/(2^(0:9)), 0.0))
stopifnot(length(betas)==12L)
# Then we do one minus this (so it is the probability of NOT being infected) and then take the log:
betas <- log(1.0 - betas)
# Populate dist so it is a matrix of log(1-betas) based on distance:
mat <- matrix(betas[case_when(as.numeric(dist) > 10 ~ 12.0, TRUE ~ as.numeric(dist) + 1.0)], ncol=ncol(dist), nrow=nrow(dist))
mat[1:5,1:5]
stopifnot(nrow(mat)==ncol(mat))

## Update function - must be done in Rust (minus the stopifnot() if you are feeling lucky):
beta_fun <- function(infected){
  # Input is the number of infected individuals per patch:
  stopifnot(length(infected)==nrow(mat))
  stopifnot(all(infected >= 0L))

  # Multiply the matrix by the vector of infected (using recycling)
  # to get the equivalent of raising to the power:
  matinf <- mat * infected
  # Then do a column sum to get the equivalent of multiplying probabilities:
  rv <- colSums(matinf)

  # Then exp and 1 minus to get the individual animal probability of being
  # infected for each patch:
  return(1.0 - exp(rv))
}

## A small test:
totalinfected <- numeric(nrow(patches))
# Infect patch ID 756 (is that the same as you???  You never sent that email!)
totalinfected[756] <- 10
patches$betas <- beta_fun(totalinfected)

ggplot(patches %>% filter(), aes(geometry=geometry, fill=log(betas))) + geom_sf()
patches %>% arrange(desc(betas)) %>% select(Index, r, q, betas)


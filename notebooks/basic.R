## Testing

library("Blofeld")

bp <- BasicPop$new(50); bp
bp$set_extbeta(c(0.1, rep(0,49)))
bp$update(1)
bp$set_extbeta(c(0, rep(0,49)))
bp$update(1)
bp$compartments

bs <- BasicSpread$new(bp, 0.99)
bs$update()
bp$update(100)

t(bp$compartments[1,,])


Scenario <- R6Class(classname = "Scenario",
                    public = list(
                      register_process = function() stop("not done")
                    ),
                    private = list(
                      #' Spread-model, authority, movement, etc.
                      processes = NULL
                    ),
                    active = NULL,
                    inherit = NULL, lock_objects = TRUE, class = TRUE,
                    portable = TRUE,
                    lock_class = FALSE, cloneable = FALSE)

Scenario$set("public", "initialize", function() {

})

Scenario$set("public", "register_process", function(process) {
  private$process <- c(private$process, process)
})

Scenario$set("public", "cache_queries", function(world) {
  stopifnot(is.world(world))

  for (process in self$processes) {
    self$regsiter_mask(process$mask_description, world)
  }
})


#' TODO: add number of ticks to progress the world.
Scenario$set("public", "run", function(world) {
  stopifnot("didn't receive a world object" = is.world(world))
  self$register_mask(process$mask_description)
  #TODO:
  #TODO: Run stack of spread models
  #TODO: Run stack of regulators models
  #TODO: Feed into authority
  for (process in private$processes) {
    process$update(world)
  }
})



Scenario$lock_class()

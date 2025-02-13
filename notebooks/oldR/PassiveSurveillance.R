

PassiveSurveillance <- R6::R6Class(classname = "PassiveSurveillance",
                                   public = list(
                                     update = function(query) {

                                     },
                                     mask_description = list()
                                   ),
                                   private = NULL,
                                   active = NULL, inherit = NULL, lock_objects = TRUE, class = TRUE,
                                   portable = TRUE, lock_class = FALSE, cloneable = TRUE)

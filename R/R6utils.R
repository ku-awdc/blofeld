`$` <- function(lhs,rhs){
  name <- deparse(substitute(rhs))
  if(is.environment(lhs) &&
      !".pointer" %in% names(lhs) &&
      !name %in% c(names(lhs),"active","private","super","deep_clone") &&
      !str_sub(name, 1L, 1L)==".")
    stop(name, " not present in environment")
  do.call(base::`$`, elem=list(lhs, name))
}

mlist <- function(..., .dummy=NULL){
  elem <- list(...)
  if(any(names(elem)=="")){
    an <- names(elem)
    an[names(elem)==""] <- "MISSING"
    stop("Empty names in member/method list: ", paste0(an,collapse=", "))
  }
  if(any(table(names(elem))>1)){
    tn <- table(names(elem))
    stop("Duplicated names in member/method list: ", paste0(names(tn)[tn>1], collapse=", "))
  }

  if(FALSE){
    for(i in which(sapply(elem, is.function))){
      args <- names(formals(elem[[i]]))
      bod <- as.character(body(elem[[i]]))
      if(length(bod)==2L && is.character(bod[2])){
        stop("Dispatch to function called bod[2] with arguments args")
      }
    }
  }

  do.call(list, elem)
}

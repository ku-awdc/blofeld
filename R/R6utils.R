`$` <- function(lhs,rhs){
  name <- deparse(substitute(rhs))
  if(is.environment(lhs) &&
      !".pointer" %in% names(lhs) &&
      !name %in% c(names(lhs),"active","private","super","deep_clone") &&
      !str_sub(name, 1L, 1L)==".")
    stop(name, " not present in environment")
  do.call(base::`$`, args=list(lhs, name))
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

  for(i in which(sapply(elem, is.function))){
    args <- names(formals(elem[[i]]))
    bod <- as.character(body(elem[[i]]))
    if(length(bod)==1L && is.character(bod)){

      # TODO:
      # 1. Improve error handling for finding function
      # 2. Auto-detect presence of self, private, super arguments (and other R6-specific??)
      # 3. Check presence (and ordering?) of other function names (without defaults?)

      stopifnot(length(find(bod))==1)
      cll <- eval(parse(text=paste0(
        "function(",
        paste(sapply(args, function(x) paste0(x, "=", x)), collapse=", "),
        "){\n",
        bod,
        "(self=self, private=private, super=super, ",
        paste(sapply(args, function(x) paste0(x, "=", x)), collapse=", "),
        ")\n}\n"
      )))
      elem[[i]] <- cll
    }
  }

  do.call(list, elem)
}

if(FALSE){
test <- function(self, private, super, x, y, z)
{
  return(x+y+z)
}
ffs <- mlist(gh = function(x,y,z) "test", gf=TRUE)
ffs$gh(1,2,3)
}

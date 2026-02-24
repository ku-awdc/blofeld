loadModule("blofeld_module", TRUE)
loadModule("blofeld_legacy_module", TRUE)

.onAttach <- function(lib, pkg)
{
  ## TODO: note API version
  # packageStartupMessage("Hello!")

}

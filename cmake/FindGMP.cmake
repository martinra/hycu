mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARY)

find_path(GMP_INCLUDE_DIR gmp.h)
find_library(GMP_LIBRARY gmp)

add_library(gmp UNKNOWN IMPORTED)
set_property(TARGET gmp PROPERTY IMPORTED_LOCATION ${GMP_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
  DEFAULT_MSG GMP_INCLUDE_DIR GMP_LIBRARY)

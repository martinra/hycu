mark_as_advanced(FLINT_INCLUDE_DIR FLINT_LIBRARY)

find_path(FLINT_INCLUDE_DIR flint/flint.h)
find_library(FLINT_LIBRARY flint)

add_library(flint UNKNOWN IMPORTED)
set_property(TARGET flint PROPERTY IMPORTED_LOCATION ${FLINT_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLINT
  DEFAULT_MSG FLINT_INCLUDE_DIR FLINT_LIBRARY)

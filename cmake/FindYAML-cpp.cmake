mark_as_advanced(YAML-cpp_INCLUDE_DIR YAML-cpp_LIBRARY)

find_path(YAML-cpp_INCLUDE_DIR yaml-cpp/yaml.h)
find_library(YAML-cpp_LIBRARY yaml-cpp)

add_library(yaml-cpp UNKNOWN IMPORTED)
set_property(TARGET yaml-cpp PROPERTY IMPORTED_LOCATION ${YAML-cpp_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YAML-cpp
  DEFAULT_MSG YAML-cpp_INCLUDE_DIR YAML-cpp_LIBRARY)

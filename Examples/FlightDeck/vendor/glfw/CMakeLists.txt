# ---- Third-party: GLFW ----
# Make sure cache options override upstream defaults
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

# Slim build: static lib, no samples/tests/install
set(BUILD_SHARED_LIBS           OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS            OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS             OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL                OFF CACHE BOOL "" FORCE)
# Force X11 on Linux (default), flip to ON for Wayland if you prefer that path:
set(GLFW_BUILD_WAYLAND          OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

# GLFW built from source defines target 'glfw'
# If you use find_package(GLFW3) you’d get 'glfw' or 'GLFW::GLFW' (imported)
if (TARGET glfw)
  add_library(glfw::glfw ALIAS glfw)
elseif(TARGET glfw3)
  add_library(glfw::glfw ALIAS glfw3)
else()
  message(WARNING "Target glfw does not exist")
endif()

set_target_properties(glfw PROPERTIES FOLDER "Examples/FlightDeck/vendor/glfw")
set_target_properties(update_mappings PROPERTIES FOLDER "Examples/FlightDeck/vendor/glfw")

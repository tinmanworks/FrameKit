# ---- Third-party: GLFW ----
# Make sure cache options override upstream defaults
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

message(STATUS "Configuring GLFW..."
  " (FRAMEKIT_VENDOR_GLFW=${FRAMEKIT_VENDOR_GLFW})")
  
# Slim build: static lib, no samples/tests/install
set(BUILD_SHARED_LIBS           OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS            OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS             OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL                OFF CACHE BOOL "" FORCE)
# Force X11 on Linux (default), flip to ON for Wayland if you prefer that path:
set(GLFW_BUILD_WAYLAND          OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

# place_in_folder(update_mappings "vendor/glfw")
# place_in_folder(glfw "vendor/glfw")

# # IDE folders and filters
if (TARGET glfw)
  set_target_properties(glfw PROPERTIES FOLDER "FrameKit/vendor/glfw")
elseif (TARGET glfw3)
  set_target_properties(glfw3 PROPERTIES FOLDER "FrameKit/vendor/glfw")
endif()
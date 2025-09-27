# ---- Third-party: GLFW ----
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

message(STATUS "===================================== Vendor GLFW ======================================")
message(STATUS "Configuring GLFW... (FRAMEKIT_VENDOR_GLFW=${FRAMEKIT_VENDOR_GLFW})")

# Vendored GLFW only if requested and not already provided
if(FRAMEKIT_VENDOR_GLFW
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/glfw/CMakeLists.txt"
   AND NOT (TARGET glfw OR TARGET glfw3 OR TARGET glfw::glfw))
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
  add_subdirectory(glfw)
endif()

# Bridge target
if(TARGET glfw OR TARGET glfw3 OR TARGET glfw::glfw)
  if(NOT TARGET FrameKitDeps.GLFW)
    add_library(FrameKitDeps.GLFW INTERFACE)
    add_library(FrameKitDeps::GLFW ALIAS FrameKitDeps.GLFW)
    set_target_properties(FrameKitDeps.GLFW PROPERTIES FOLDER "FrameKitDeps")
  endif()
  if(TARGET glfw)
    set_target_properties(glfw PROPERTIES FOLDER "FrameKitDeps/glfw")
    set_target_properties(update_mappings PROPERTIES FOLDER "FrameKitDeps/glfw")
    target_link_libraries(FrameKitDeps.GLFW INTERFACE
      $<BUILD_INTERFACE:glfw>
      $<INSTALL_INTERFACE:glfw::glfw>)
  elseif(TARGET glfw3)
    set_target_properties(glfw3 PROPERTIES FOLDER "FrameKitDeps/glfw")
    set_target_properties(update_mappings PROPERTIES FOLDER "FrameKitDeps/glfw")
    target_link_libraries(FrameKitDeps.GLFW INTERFACE
      $<BUILD_INTERFACE:glfw3>
      $<INSTALL_INTERFACE:glfw::glfw>)
  else()
    set_target_properties(glfw::glfw PROPERTIES FOLDER "FrameKitDeps/glfw")
    set_target_properties(update_mappings PROPERTIES FOLDER "FrameKitDeps/glfw")
    target_link_libraries(FrameKitDeps.GLFW INTERFACE glfw::glfw)
  endif()
endif()

message(STATUS "========================================================================================")
# ---- End Third-party: GLFW ----

# Export shim only if it exists
if(TARGET FrameKitDeps.GLFW)
  install(TARGETS FrameKitDeps.GLFW
    EXPORT FrameKitDepsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/$<CONFIG>
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}/$<CONFIG>
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}/$<CONFIG>)
  install(EXPORT FrameKitDepsTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FrameKit)
endif()
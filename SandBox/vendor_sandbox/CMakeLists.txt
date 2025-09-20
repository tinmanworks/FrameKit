
if (NOT TARGET Glad)
    message(STATUS "Adding Glad subdirectory")
    add_subdirectory(Glad)
endif()

if (NOT TARGET glfw)
    message(STATUS "Adding GLFW subdirectory")
    add_subdirectory(glfw)
endif()

if (NOT TARGET imgui)
    message(STATUS "Adding ImGui subdirectory")
    add_subdirectory(imgui)
endif()

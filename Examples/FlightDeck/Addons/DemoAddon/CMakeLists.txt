cmake_minimum_required(VERSION 3.20)

project(Addon_Demo)

add_library(Addon_Demo SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/Addon_Demo.cpp
)

target_include_directories(Addon_Demo PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Addon_Demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/)
target_compile_definitions(Addon_Demo PRIVATE FRAMEKIT_BUILDING_ADDON)

target_link_libraries(Addon_Demo PRIVATE imgui)


# Optional: only if you really need imgui inside the addon
# target_link_libraries(Addon_Demo PRIVATE imgui)

# Emit a plugin-looking file
set_target_properties(Addon_Demo PROPERTIES
  OUTPUT_NAME "addon_demo"
  SUFFIX ".sae"
  FOLDER "Examples/FlightDeck/Addons"
)

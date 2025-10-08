cmake_minimum_required(VERSION 3.23)

project(FlightDeck
  VERSION 0.1.0
  DESCRIPTION "FlightDeck application"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output dirs (build tree)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(vendor)

file(GLOB_RECURSE flightdeck_files
        src/*.cpp
        src/*.h
)

add_executable(FlightDeck)

target_sources(FlightDeck PRIVATE
    ${flightdeck_files}
)

target_link_libraries(FlightDeck 
FrameKit::FrameKit
FrameKit::Window.GLFW
imgui
)

if(WIN32)
    target_link_libraries(FlightDeck FrameKit::Window.Win32 )
endif()


set_target_properties(FlightDeck PROPERTIES FOLDER "Examples/FlightDeck")

add_subdirectory(Addons)
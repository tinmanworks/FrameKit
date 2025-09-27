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

add_executable(FlightDeck)

target_sources(FlightDeck PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/src/FlightDeckApp.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/src/ImGui/ImGuiLayer.h"
  "${CMAKE_CURRENT_LIST_DIR}/src/ImGui/ImGuiLayer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/src/DemoLayer.h"
  "${CMAKE_CURRENT_LIST_DIR}/src/VideoLayer.h"
  "${CMAKE_CURRENT_LIST_DIR}/src/DemoLayer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/src/VideoLayer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/src/utils/FileDialog.h"
  "${CMAKE_CURRENT_LIST_DIR}/src/utils/FileDialog.cpp"
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
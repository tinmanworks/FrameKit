cmake_minimum_required(VERSION 3.23)

project(SandBox
  VERSION 0.1.0
  DESCRIPTION "SandBox application"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output dirs (build tree)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


add_executable(SandBox)

target_sources(SandBox PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/src/SandBoxApp.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/src/DemoLayer.h"
  "${CMAKE_CURRENT_LIST_DIR}/src/DemoLayer.cpp"
)

target_link_libraries(SandBox FrameKit::FrameKit)

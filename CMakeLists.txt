cmake_minimum_required(VERSION 3.22)

# Specify vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE 
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

project(small_gl LANGUAGES CXX)

# Include third party libraries provided through vcpkg
find_package(Eigen3 CONFIG REQUIRED)
find_package(glad   CONFIG REQUIRED)
find_package(glfw3  CONFIG REQUIRED)
find_package(fmt    CONFIG REQUIRED)

# Include third party libraries provided directly
add_subdirectory(third_party/small_gl_parser)

# Recursively gather files
file(GLOB_RECURSE srcs 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/small_gl/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Configure small_gl library build target
add_library(small_gl ${srcs})
target_link_libraries(small_gl PRIVATE
  Eigen3::Eigen
  glad::glad
  glfw
  fmt::fmt-header-only
  small_gl_parser_lib
)
target_include_directories(small_gl PUBLIC include)
target_compile_features(small_gl PRIVATE cxx_std_20)

cmake_minimum_required(VERSION 3.22)

# Specify vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE 
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

project(small_gl LANGUAGES CXX)

# Build options
option(gl_enable_dbg_exceptions "Enable debug exceptions on release" OFF)
option(gl_enable_tracy          "Enable Tracy support"               OFF)

# Include third party libraries provided through vcpkg
find_package(Eigen3        CONFIG REQUIRED)
find_package(glad          CONFIG REQUIRED)
find_package(glfw3         CONFIG REQUIRED)
find_package(fmt           CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Tracy         CONFIG REQUIRED)

# Include third party libraries provided directly
option(glp_enable_dbg_exceptions "" ${gl_enable_dbg_exceptions})
add_subdirectory(third_party/small_gl_parser)

# Configuration output info
message(STATUS "small_gl  : Enabling exceptions = ${gl_enable_dbg_exceptions}")
message(STATUS "small_gl  : Enabling Tracy      = ${gl_enable_tracy}")

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
  nlohmann_json::nlohmann_json
  small_gl_parser_lib
  Tracy::TracyClient
)
target_include_directories(small_gl PUBLIC include)
target_compile_features(small_gl PRIVATE cxx_std_20)

# Configure build options
if (gl_enable_dbg_exceptions)
  target_compile_definitions(small_gl PUBLIC GL_ENABLE_DBG_EXCEPTIONS)
endif()
if (gl_enable_tracy)
  target_compile_definitions(small_gl PUBLIC GL_ENABLE_TRACY)
endif()

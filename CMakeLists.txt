cmake_minimum_required(VERSION 3.22)

# Specify vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE 
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

# Specify project name, languages
project(small_gl LANGUAGES CXX)

# Build options
option(gl_enable_exceptions "Enable debug exceptions on release" OFF)
option(gl_enable_tracy      "Enable Tracy support"               OFF)

# Include third party libraries provided through vcpkg
find_package(Eigen3        CONFIG REQUIRED)
find_package(fmt       CONFIG REQUIRED)
find_package(glad          CONFIG REQUIRED)
find_package(glfw3         CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Tracy         CONFIG REQUIRED)
find_package(ZLIB          REQUIRED)

# Include third party header-only libraries provided through vcpkg
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
find_path(ZSTR_INCLUDE_DIRS "zstr.hpp")

# Configuration output info
message(STATUS "small_gl  : Enabling exceptions = ${gl_enable_exceptions}")
message(STATUS "small_gl  : Enabling Tracy      = ${gl_enable_tracy}")

# Recursively gather source files
file(GLOB_RECURSE srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE hdrs ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

# Configure small_gl library build target
add_library(small_gl ${srcs})
target_link_libraries(small_gl PRIVATE
  Eigen3::Eigen
  glad::glad
  glfw
  fmt::fmt-header-only
  nlohmann_json::nlohmann_json
  Tracy::TracyClient
  ZLIB::ZLIB
)
target_include_directories(small_gl PUBLIC include ${ZSTR_INCLUDE_DIRS})
target_compile_features(small_gl PRIVATE cxx_std_23)
target_precompile_headers(small_gl PUBLIC ${hdrs})

# Enable /bigobj to make MSVC stop complaining about its restricted object sizes
if (MSVC)
  target_compile_options(small_gl PRIVATE /bigobj)
endif()

# Configure build options
if (gl_enable_exceptions)
  target_compile_definitions(small_gl PUBLIC GL_ENABLE_EXCEPTIONS)
endif()
if (gl_enable_tracy)
  target_compile_definitions(small_gl PUBLIC GL_ENABLE_TRACY)
endif()

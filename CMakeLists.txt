# CMake Configuration for sQeeZ-Parser
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the project name
project(sQeeZ-Parser)

# OS specific configurations
if(MSVC)
  add_definitions(-D_SQEEZ_WIN)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(APPLE)
  add_definitions(-D_SQEEZ_MAC)
else()
  add_definitions(-D_SQEEZ_LINUX)
endif()

# Include the header directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Include Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# Configure Google Test
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
include(GoogleTest)

# Include the source files
file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Include the header files
file(GLOB_RECURSE HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

# Include the Lexer
set(SQEEZ_LEXER_LIB_PATH ${CMAKE_SOURCE_DIR}/lexer-lib/libsQeeZ-Lexer-Lib.a)
set(SQEEZ_LEXER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/lexer)
include_directories(${SQEEZ_LEXER_INCLUDE_DIR})
add_library(sQeeZ-Lexer-Lib STATIC IMPORTED)
set_target_properties(sQeeZ-Lexer-Lib PROPERTIES IMPORTED_LOCATION ${SQEEZ_LEXER_LIB_PATH})

# Create the Library
add_library(
  sQeeZ-Parser-Lib 
  STATIC 
  ${SOURCES} 
  ${HEADERS}
)

# Create the Executable
add_executable(
  sQeeZ-Parser-Exe
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
target_link_libraries(
  sQeeZ-Parser-Exe 
  sQeeZ-Parser-Lib
  sQeeZ-Lexer-Lib
)

# Parser Test
add_executable(
  parser_test 
  ${CMAKE_CURRENT_SOURCE_DIR}/test/parser_test.cc
)
target_link_libraries(
  parser_test
  GTest::gtest_main
  sQeeZ-Parser-Lib
  sQeeZ-Lexer-Lib
)
gtest_discover_tests(parser_test)

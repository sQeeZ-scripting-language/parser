# CMake Configuration for sQeeZ-Parser
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=1)

# Set the project name
project(sQeeZ-Parser)

# OS specific configurations
if(MSVC)
  add_definitions(-D_SQEEZ_WIN)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
elseif(APPLE)
  add_definitions(-D_SQEEZ_MAC)
  set(CMAKE_OSX_ARCHITECTURES "arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
elseif(UNIX)
  add_definitions(-D_SQEEZ_LINUX)
else()
  message(FATAL_ERROR "Unsupported Operating System")
endif()

# Print the compiler information
message(STATUS "Operating System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Compiler Version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

# Include the header directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Include NodeJS
include(./node_modules/node-cmake/NodeJS.cmake)
nodejs_init()

# After init, we will have headers required at the following folder:
# ${CMAKE_CURRENT_BINARY_DIR}/node/${NODEJS_VERSION}/include
include_directories(
  ${CMAKE_JS_INC} 
  "${CMAKE_CURRENT_BINARY_DIR}/node/${NODEJS_VERSION}/include" 
  "${CMAKE_HOME_DIRECTORY}/node_modules/node-addon-api"
)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/node/*.cpp
)

# Include the header files
file(GLOB_RECURSE HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/parser/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/parser/node/*.hpp
)

# Include the Lexer
if(MSVC)
  set(SQEEZ_LEXER_LIB_PATH ${CMAKE_SOURCE_DIR}/lexer-lib/sQeeZ-Lexer-Lib.lib)
  add_library(sQeeZ-Lexer-Lib STATIC IMPORTED)
  set_target_properties(sQeeZ-Lexer-Lib PROPERTIES
    IMPORTED_LOCATION ${SQEEZ_LEXER_LIB_PATH}
    IMPORTED_IMPLIB ${SQEEZ_LEXER_LIB_PATH}
  )
else()
  set(SQEEZ_LEXER_LIB_PATH ${CMAKE_SOURCE_DIR}/lexer-lib/libsQeeZ-Lexer-Lib.a)
  add_library(sQeeZ-Lexer-Lib STATIC IMPORTED)
  set_target_properties(sQeeZ-Lexer-Lib PROPERTIES IMPORTED_LOCATION ${SQEEZ_LEXER_LIB_PATH})
endif()
set(SQEEZ_LEXER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/lexer)
include_directories(${SQEEZ_LEXER_INCLUDE_DIR})

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

# Create the NodeJS module
if(${NODE})
  add_library(
    sQeeZ-Parser-Node 
    SHARED 
    ${SOURCES} 
    ${HEADERS}
  )
  set_target_properties(
    sQeeZ-Parser-Node 
    PROPERTIES PREFIX "" SUFFIX ".node"
  )
  target_link_libraries(
    sQeeZ-Parser-Node 
    ${CMAKE_JS_LIB}
    sQeeZ-Parser-Lib
    sQeeZ-Lexer-Lib
  )
endif()
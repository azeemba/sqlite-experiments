cmake_minimum_required(VERSION 3.14)
project(SqliteExperiment)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(sqexp main.cpp sqlite_wrap.cpp)
target_link_libraries(sqexp ${CONAN_LIBS})
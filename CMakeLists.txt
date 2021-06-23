
cmake_minimum_required(VERSION 3.0)

project(yecc)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11 -pthread") 

aux_source_directory(${PROJECT_SOURCE_DIR}/src LIB_SRCS)  
add_library(yecc SHARED ${LIB_SRCS})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(~/boost_1_76_0)
include_directories(~/yaml-cpp/include)

find_library(YAMLCPP libyaml-cpp.a)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib) 

set(EXE_FILE test_thread)

add_executable(${EXE_FILE} ${PROJECT_SOURCE_DIR}/test/${EXE_FILE}.cpp)
add_dependencies(${EXE_FILE} yecc)
target_link_libraries(${EXE_FILE} yecc ${YAMLCPP})
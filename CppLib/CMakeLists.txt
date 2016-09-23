cmake_minimum_required(VERSION 3.5)
project(CppLib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set(SOURCE_FILES main.cpp HashTable.h Vector.h Snake.cpp Snake.h)
add_executable(CppLib ${SOURCE_FILES} HashTable.h Vector.h Snake.cpp Snake.h)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(main.cpp ${OpenCV_LIBS}) # replace binary_name


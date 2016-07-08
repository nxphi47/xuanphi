cmake_minimum_required(VERSION 3.5)
project(RaspiLearn)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# set(CMAKE_CXX_FLAGS "-pthread -I/usr/local/include -L/usr/local/lib -lwiringPi")
SET(CMAKE_CXX_FLAGS "-pthread -lwiringPi")

set(SOURCE_FILES main.cpp /usr/local/include/wiringPi.h TestLib.h)
add_executable(RaspiLearn ${SOURCE_FILES} TestLib.h)
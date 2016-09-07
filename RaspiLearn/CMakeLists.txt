cmake_minimum_required(VERSION 3.5)
project(RaspiLearn)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# set(CMAKE_CXX_FLAGS "-pthread -I/usr/local/include -L/usr/local/lib -lwiringPi")
SET(CMAKE_CXX_FLAGS "-pthread -lwiringPi")
#SET(CMAKE_CXX_FLAGS "-lwiringPi")

link_libraries(/usr/local/lib/libwiringPi.so)

# Locate libraries and headers
# find_package(WiringPi REQUIRED)
# find_package(Threads REQUIRED)
# include them
# include_directories(${WIRINGPI_INCLUDE_DIRS})
# linking
# target_link_libraries(${WIRINGPI_LIBRARIES})
# target_link_libraries(${CMAKE_THREAD_LIBS_INIT})
# /usr/local/include/wiringPi.h

# Locate libraries and headers
# find_package(WiringPi REQUIRED)
# find_package(Threads REQUIRED)

# Include headers
# include_directories(${WIRINGPI_INCLUDE_DIRS})

# Link against libraries
# target_link_libraries(${WIRINGPI_LIBRARIES})
# target_link_libraries(${CMAKE_THREAD_LIBS_INIT})

set(SOURCE_FILES main.cpp TestLib.h)
add_executable(RaspiLearn ${SOURCE_FILES} TestLib.h)
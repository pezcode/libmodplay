cmake_minimum_required(VERSION 2.8)

project(libmodplayTest)

add_executable(libmodplayTest main.cpp)
add_subdirectory(libmodplay)

if(CMAKE_CL_64)
	set(LIBMODPLUG_LIB "libmodplug64")
else()
	set(LIBMODPLUG_LIB "libmodplug")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libmodplay)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libmodplug)

target_link_libraries(libmodplayTest libmodplay)
target_link_libraries(libmodplayTest libmodplug/${LIBMODPLUG_LIB})
target_link_libraries(libmodplayTest winmm)

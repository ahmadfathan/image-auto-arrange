cmake_minimum_required(VERSION 3.1)

project(
    AutoArrangeImage
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package( OpenCV REQUIRED )

include_directories( 
    ${OpenCV_INCLUDE_DIRS} 
)

add_library(
    mylib STATIC 
    lib/ga/individual.h
    lib/ga/individual.cpp
    lib/ga/population.h
    lib/ga/population.cpp
    lib/utils.h
    lib/utils.cpp
)

link_libraries(
    opencv_core 
    opencv_imgcodecs 
    opencv_imgproc 
    opencv_highgui
)

add_executable(main main.cpp)
add_executable(test_rotate test_rotate.cpp)

target_link_libraries(main PRIVATE mylib)

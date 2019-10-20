cmake_minimum_required( VERSION 2.6 )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../Modules/")

add_library( current_control current_control.cpp )

find_package( pigpio REQUIRED )

target_link_libraries( current_control
    ${pigpio_LIBRARY}
    i2c
    )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../i2c )

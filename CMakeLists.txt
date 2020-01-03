set ( VERBOSE=1 )
cmake_minimum_required( VERSION 2.6 )
project( ledwul )
set( SOURCE_FILES amp_new.cpp )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Wpedantic")

add_executable( ledwul ${SOURCE_FILES} )

include_directories( ${PROJECT_SOURCE_DIR}/current_control )
link_directories( ${PROJECT_SOURCE_DIR}/current_control )
add_subdirectory( current_control )

include_directories( ${PROJECT_SOURCE_DIR}/i2c )
link_directories( ${PROJECT_SOURCE_DIR}/i2c )
add_subdirectory( i2c )

find_package( pigpio REQUIRED )
find_package( Threads REQUIRED )

target_link_libraries( ledwul
  ${pigpio_LIBRARY}
  Threads::Threads
  i2c
  current_control
  )

# Stop executable

add_executable( ledwul_stop amp_stop.cpp )

target_link_libraries( ledwul_stop
  ${pigpio_LIBRARY}
  Threads::Threads
  i2c
  current_control
  )

# New main file

add_executable( ledwul_new main.cpp )

target_link_libraries( ledwul_new
  ${pigpio_LIBRARY}
  Threads::Threads
  i2c
  current_control
  )

target_compile_options(ledwul_new PRIVATE -Werror -Wall -Wextra)

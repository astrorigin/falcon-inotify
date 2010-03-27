##
#   Falcon inotify project
#
project( inotify )

cmake_minimum_required( VERSION 2.8 )

##
#   Check for a Linux system
#
if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    message ( FATAL_ERROR "Linux system required, sorry." )
endif ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )

##
#   Find Falcon
#
find_package( Falcon
    HINTS "$ENV{FALCON_ACTIVE_TREE}/*/*/cmake"
          "$ENV{FALCON_ACTIVE_TREE}/*/cmake"
          "$ENV{FALCON_BIN_PATH}/cmake"
    REQUIRED )
include_directories( ${Falcon_INCLUDE_DIRS} )
link_directories( ${FALCON_LIB_PATH} )

##
#   Find inotify.
#   Should check information about the running kernel instead.
#
find_file( INOTIFY_HEADER sys/inotify.h
    PATHS /usr/include )

##
#   Subdirs
#
add_subdirectory( src )

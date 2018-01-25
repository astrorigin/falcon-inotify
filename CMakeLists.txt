##
#   Falcon inotify project
#
project( falcon_inotify )

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

##
#   Find inotify.
#   Should check information about the running kernel instead.
include(CheckIncludeFile)
check_include_file("sys/inotify.h" HAVE_SYS_INOTIFY_H)
if(NOT HAVE_SYS_INOTIFY_H)
  message(FATAL_ERROR "This Linux installation doesn't got inotify.")
endif()

##
#   Subdirs
#
add_subdirectory( src )

# vi: set fenc=utf-8 ff=unix et sw=2 ts=2 sts=2 :

##
#   Falcon inotify module
#

falcon_define_module( CURRENT_MODULE inotify )

##
#   Sources
#
set( SRC_FILES
    modinotify.cpp
    modinotify_st.cpp
    )

##
#   Header files.  If not added to the target source, they won't show
#   up in Visual Studio.  :P
#
set( HDR_FILES
    modinotify.hpp
    modinotify_st.hpp
    modinotify_version.hpp
    )

##
#   Target
#
add_library( ${CURRENT_MODULE} MODULE
    ${SRC_FILES}
    ${HDR_FILES}
    )

##
#   Linking
#
target_link_libraries( ${CURRENT_MODULE}
    ${Falcon_LIBRARIES}
    )

##
#   Install
#
falcon_install_module( ${CURRENT_MODULE} )

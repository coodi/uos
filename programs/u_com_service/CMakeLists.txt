add_executable( u_com_service main.cpp )

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling eosio-blocklog with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

#add_library(
#
#        SimplePocoHandler.cpp
#        SimplePocoHandler.h
#)

target_include_directories(u_com_service PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries( u_com_service
        PRIVATE appbase
        PRIVATE eosio_chain fc  singularity amqpcpp PocoNet PocoFoundation u_com ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )

install( TARGETS
        u_com_service

   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
)

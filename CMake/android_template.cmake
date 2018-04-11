SET( MENGINE_LIB_PREFIX "lib" CACHE STRING "MENGINE_LIB_PREFIX" FORCE )
SET( MENGINE_LIB_SUFFIX ".a" CACHE STRING "MENGINE_LIB_SUFFIX" FORCE )
SET( MENGINE_SO_SUFFIX ".so" CACHE STRING "MENGINE_SO_SUFFIX" FORCE )

SET( CMAKE_DEBUG_POSTFIX "" CACHE STRING "Set debug library postfix" FORCE )

#SET( CMAKE_C_FLAGS "-std=c11 -Wa,-mimplicit-it=thumb" )
#SET( CMAKE_CXX_FLAGS "-std=c++14 -frtti -fexceptions -Wa,-mimplicit-it=thumb" )

set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG" )
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} -DNDEBUG" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -DNDEBUG" )
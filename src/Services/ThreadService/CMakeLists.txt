SET( MY_LIB_NAME ThreadService )

ADD_FILTER(
src
	ThreadService.h
	ThreadService.cpp
    ThreadQueue.h
    ThreadQueue.cpp    
    ThreadMutexDummy.h
)

ADD_MENGINE_LIBRARY()

set_target_properties (${MY_LIB_NAME} PROPERTIES
    FOLDER Services
)

INCLUDE_DIRECTORIES( 
    ${THIRDPARTY_DIR}/libmath/include
    ) 

SET( APPLICATION_DEPENDENCIES
 ${APPLICATION_DEPENDENCIES}
 ${MY_LIB_NAME}
)


SET( APPLICATION_DEPENDENCIES
 ${APPLICATION_DEPENDENCIES}
 PARENT_SCOPE
)

set( MY_LIB_NAME DX9RenderSystem )

ADD_FILTER(
src
	DX9RenderSystem.h
	DX9RenderSystem.cpp
	DX9RenderImage.h
	DX9RenderImage.cpp
	DX9RenderImageTarget.h
	DX9RenderImageTarget.cpp
    DX9RenderTargetTexture.h
    DX9RenderTargetTexture.cpp
	DX9RenderTargetOffscreen.h
	DX9RenderTargetOffscreen.cpp
    DX9RenderVertexBuffer.h
    DX9RenderVertexBuffer.cpp
    DX9RenderVertexAttribute.h
    DX9RenderVertexAttribute.cpp
    DX9RenderIndexBuffer.h
    DX9RenderIndexBuffer.cpp    
    DX9RenderFragmentShader.h
    DX9RenderFragmentShader.cpp
    DX9RenderVertexShader.h
    DX9RenderVertexShader.cpp
    DX9RenderProgram.h
    DX9RenderProgram.cpp
    DX9RenderEnum.h
    DX9ErrorHelper.h
    DX9ErrorHelper.cpp
    DX9RenderProgramVariable.h
    DX9RenderProgramVariable.cpp
)

INCLUDE_DIRECTORIES( ${THIRDPARTY_DIR}/libmath/include )

ADD_MENGINE_LIBRARY()

set_target_properties (${MY_LIB_NAME} PROPERTIES
    FOLDER Systems
)

SET( APPLICATION_DEPENDENCIES
    ${APPLICATION_DEPENDENCIES}
    ${MY_LIB_NAME}
)
    
SET( APPLICATION_DEPENDENCIES
 ${APPLICATION_DEPENDENCIES}
 PARENT_SCOPE
)

SET( APPLICATION_LIB_PATHS
 ${APPLICATION_LIB_PATHS}
 PARENT_SCOPE
)

SET( APPLICATION_LIBS
 ${APPLICATION_LIBS}  
)

SET( APPLICATION_LIBS
 ${APPLICATION_LIBS}  
 PARENT_SCOPE
)

# SET LIBNAME
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(MENGINE_IPAD_SIMULATOR)

SET (CMAKE_CROSSCOMPILING   TRUE)
SET (CMAKE_SYSTEM_NAME      "Darwin")
SET (CMAKE_SYSTEM_PROCESSOR "arm")

set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "2")

SET(SDK_NAME iPhoneSimulator)
SET(SDK_VERSION 5.0)

SET(TARGETSDK_VERSION 3.2)

#SET( TARGETSDK_NAME iPhoneOS )
#SET( TARGETSDK_VERSION 5.0 )

SET(DEVROOT /Developer/Platforms/${SDK_NAME}.platform/Developer)
SET(SDKROOT ${DEVROOT}/SDKs/${SDK_NAME}${SDK_VERSION}.sdk)
SET(CMAKE_OSX_SYSROOT ${SDKROOT})

SET(CMAKE_FRAMEWORK_PATH ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks)

INCLUDE_DIRECTORIES ( ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks )

SET(CMAKE_OSX_ARCHITECTURES "armv6" "armv7")
set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")

SET (CMAKE_C_COMPILER   "${DEVROOT}/usr/bin/clang")
SET (CMAKE_CXX_COMPILER "${DEVROOT}/usr/bin/clang++")

#SET (CMAKE_C_FLAGS      "-fshort-wchar")
#SET (CMAKE_CXX_FLAGS    "-fshort-wchar")


#SET (CMAKE_EXE_LINKER_FLAGS    "-miphoneos-version-min=${TARGETSDK_VERSION}")
#SET (CMAKE_SHARED_LINKER_FLAGS "-miphoneos-version-min=${TARGETSDK_VERSION}")
#SET (CMAKE_MODULE_LINKER_FLAGS "-miphoneos-version-min=${TARGETSDK_VERSION}")

#SET (CMAKE_FIND_ROOT_PATH              "${SDKROOT}" "${DEVROOT}")
#SET (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#SET (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)




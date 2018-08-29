cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
#cmake_policy(SET CMP0011 OLD)

project (AtracsysWrapper)

# On Windows, we look for the installation folder in the registry
IF(WIN32)
    GET_FILENAME_COMPONENT(ATRACSYS_SDK_INSTALL_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Atracsys\\spryTrack;Root]" ABSOLUTE  )
ELSE()
# On Linux, we assume that the installation folder is on directory below.
    SET(ATRACSYS_SDK_INSTALL_PATH ../..)
ENDIF()
MESSAGE(${ATRACSYS_SDK_INSTALL_PATH})

set(ATRACSYS_LIB_DIR "${ATRACSYS_SDK_INSTALL_PATH}/lib")
set(ATRACSYS_INCLUDE_DIR "${ATRACSYS_SDK_INSTALL_PATH}/include")
set(ATRACSYS_BIN_DIR "${ATRACSYS_SDK_INSTALL_PATH}/bin")
ADD_DEFINITIONS( -DATR_FTK )


configure_file(${ATRACSYS_BIN_DIR}/fusionTrack64.dll ${CMAKE_BINARY_DIR}/fusionTrack64.dll COPYONLY)
configure_file(${ATRACSYS_BIN_DIR}/libusb-1.0.dll ${CMAKE_BINARY_DIR}/libusb-1.0.dll COPYONLY)
configure_file(${ATRACSYS_BIN_DIR}/device64.dll ${CMAKE_BINARY_DIR}/device64.dll COPYONLY)
configure_file(${ATRACSYS_BIN_DIR}/freeglut.dll ${CMAKE_BINARY_DIR}/freeglut.dll COPYONLY)

SET(ARCH 64)
SET(LIBS "${ATRACSYS_LIB_DIR}/fusionTrack${ARCH}.lib" )


## attracsys library
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(atracsyswrapper SHARED
        lib/src/atracsyswrapperimpl.cpp
        lib/src/atracsysdevice.cpp lib/src/atracsysdevice.h
        lib/src/atracsysmarker.cpp lib/include/atracsyswrapper/atracsysmarker.h
        lib/src/helpers_windows.cpp lib/include/atracsyswrapper/atracsyswrapper.h
        lib/src/atracsyswrapper.cpp lib/include/atracsyswrapper/atracsysmarker.h)

target_include_directories(atracsyswrapper PUBLIC lib/include)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(${ATRACSYS_INCLUDE_DIR})
target_link_libraries(atracsyswrapper ${LIBS})

#
#
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()
#
##test app
#add_executable(testapps test/testapp.cpp lib/src/helpers.hpp lib/src/helpers_windows.cpp lib/src/version.cpp lib/src/geometryHelper.hpp)
#
## Test
#add_executable(wrappertest
#        test/wrappertest.cpp test/connectionlistener.cpp test/connectionlistener.h)
#target_link_libraries(wrappertest atracsyswrapper)
#
##IF (WIN32)
##    # On Windows, we use the delay loading of DLLs along with the function
##    # SetDllDirectory() to load dlls outside of the Executable folder.
##    SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " /DELAYLOAD:fusionTrack${ARCH}.dll")
##    #ADD_DEFINITIONS(-DFORCED_DEVICE_DLL_PATH="${ATRACSYS_SDK_INSTALL_PATH}/bin")
##    SET( LIBS ${LIBS} delayimp )
##
##    SET( LIBS ${LIBS} Winmm )
##    IF (MSVC_IDE)
##        SET( LIBS ${LIBS} optimized MSVCRT )
##        SET( LIBS ${LIBS} debug MSVCRTD )
##    ENDIF (MSVC_IDE)
##ENDIF (WIN32)
#
#MESSAGE(${LIBS})
#
#
#
#target_link_libraries(testapps ${LIBS})
#
#target_link_libraries(wrappertest ${CONAN_LIBS_OPENIGTLINK})

# CMake script for building XStxExampleServer

cmake_minimum_required (VERSION 2.6)

project (XStxDirectXServer)

# Determine target architecture.  This is set depending on whether
#  vcvarsall.bat was called with "amd64" argument or not.
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (Architecture x64)
    set (ArchSuffix x64)
    message("\nTarget architecture is x64\n") 
else()
    set (Architecture x86)
    set (ArchSuffix "")
    message("\nTarget architecture is x86\n") 
endif()

# Include and library paths set if not already defined.
if ( NOT DEFINED XSTX_LIB_DIR)
   set (XSTX_LIB_DIR "../../../../lib/windows/${Architecture}")
endif() 

if ( NOT DEFINED XSTX_INCLUDE_DIR)
   set (XSTX_INCLUDE_DIR "../../../../include") 
endif()

# Preprocessor definitions

add_definitions(-DAPPSTREAM_GAME)

# Paths to headers for external libs

include_directories ("${XSTX_INCLUDE_DIR}")

# Paths to headers internal to the example

# common to client and server examples
include_directories ("${PROJECT_SOURCE_DIR}/../../../common")
# common to server examples
include_directories ("${PROJECT_SOURCE_DIR}/../../common")

if (WIN32)
    # common to server examples for windows only
    include_directories ("${PROJECT_SOURCE_DIR}/../../common/windows")
endif()

# DirectX example works under Windows environment only
if(WIN32)

    # Generate debug info
    set (CMAKE_CXX_FLAGS "/Zi ${CMAKE_CXX_FLAGS}")
    set (CMAKE_C_FLAGS "/Zi ${CMAKE_C_FLAGS}")

    # On windows, include DirectX
    if (DEFINED ENV{DXSDK_DIR})
        include_directories("$ENV{DXSDK_DIR}Include")
    endif()

    # DirectX example
    set (SRCS_DIRECTX 
        ../../common/ServerManagerListener.cpp
        ../../common/XStxExampleServer.cpp
        ../../common/windows/Game/GameWindow.cpp
        ../../../common/MUD/base/TimeVal.cpp
        ../../../common/MUD/base/windows/WindowsTimeVal.cpp
        ../../common/Color/ColorConversions.cpp
        ../../common/windows/Capture/GDICapture.cpp
        ../../common/windows/Capture/DXCapture.cpp
        ../../common/windows/Audio/Audio.cpp
        HostedApplicationDirectX.cpp
        Game.cpp)
        
endif(WIN32)

get_filename_component (XSTX_LIB_DIR "${XSTX_LIB_DIR}" REALPATH)
link_directories ("${XSTX_LIB_DIR}")


if(WIN32)
    add_executable (XStxDirectXServer ${SRCS_DIRECTX})
    target_link_libraries (XStxDirectXServer XStxServerLibraryShared${ArchSuffix})
    target_link_libraries (XStxDirectXServer winmm d3d11 dxgi xapobase)
endif (WIN32)

install (TARGETS XStxDirectXServer DESTINATION XStxDirectXServer)
install (DIRECTORY resources DESTINATION XStxDirectXServer)


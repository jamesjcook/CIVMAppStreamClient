cmake_minimum_required (VERSION 2.8)

PROJECT (StxExampleClients)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (Architecture x64)
else()
    set (Architecture x86)
endif()

set (STX_EXAMPLE_CLIENTS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../src")

set (FFMPEG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/ffmpeg/include")
set (FFMPEG_LIB_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/ffmpeg/lib/${Architecture}")
set (OPUS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/opus/include")
set (OPUS_LIB_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/opus/lib/${Architecture}")
set (PORTAUDIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/portaudio/include")
set (PORTAUDIO_LIB_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/portaudio/lib/${Architecture}")
set (MS_INT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../../../3rdparty/windows/msinttypes")
set (XSTX_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../../../../include")
set (XSTX_LIB_DIR "${PROJECT_SOURCE_DIR}/../../../../lib/windows/${Architecture}")
set (XSTX_EXAMPLES_DIR "${PROJECT_SOURCE_DIR}../../../../")
set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/build")


get_filename_component (FFMPEG_INCLUDE_DIR "${FFMPEG_INCLUDE_DIR}" REALPATH)
get_filename_component (FFMPEG_LIB_DIR "${FFMPEG_LIB_DIR}" REALPATH)

get_filename_component (OPUS_INCLUDE_DIR "${OPUS_INCLUDE_DIR}" REALPATH)
get_filename_component (OPUS_LIB_DIR "${OPUS_LIB_DIR}" REALPATH)

get_filename_component (PORTAUDIO_INCLUDE_DIR "${PORTAUDIO_INCLUDE_DIR}" REALPATH)
get_filename_component (PORTAUDIO_LIB_DIR "${PORTAUDIO_LIB_DIR}" REALPATH)

get_filename_component (MS_INT_INCLUDE_DIR "${MS_INT_INCLUDE_DIR}" REALPATH)

get_filename_component (XSTX_INCLUDE_DIR "${XSTX_INCLUDE_DIR}" REALPATH)
get_filename_component (XSTX_LIB_DIR "${XSTX_LIB_DIR}" REALPATH)
get_filename_component (XSTX_EXAMPLES_DIR "${XSTX_EXAMPLES_DIR}" REALPATH)

# Paths to headers for external libs

include_directories ("${FFMPEG_INCLUDE_DIR}")
include_directories ("${OPUS_INCLUDE_DIR}")
include_directories ("${PORTAUDIO_INCLUDE_DIR}")
include_directories ("${MS_INT_INCLUDE_DIR}")
include_directories ("${XSTX_INCLUDE_DIR}")

set (XSTX_EXAMPLES_COMMON "${XSTX_EXAMPLES_DIR}/common")
set (XSTX_EXAMPLES_ACR "${XSTX_EXAMPLES_COMMON}/AmazonCompositeResult")
set (XSTX_EXAMPLES_MUD "${XSTX_EXAMPLES_COMMON}/MUD")

include_directories ("${XSTX_EXAMPLES_COMMON}")
include_directories ("${XSTX_EXAMPLES_ACR}")
include_directories ("${XSTX_EXAMPLES_MUD}")

link_directories ("${FFMPEG_LIB_DIR}")
link_directories ("${OPUS_LIB_DIR}")
link_directories ("${PORTAUDIO_LIB_DIR}")
link_directories ("${XSTX_LIB_DIR}")


include_directories ("${STX_EXAMPLE_CLIENTS_SOURCE_DIR}")
include_directories ("${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/opus_decoder")
include_directories ("${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/ffmpeg_decoder")
include_directories ("${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/portaudio_renderer")

set (ACR_SRCS
    ${XSTX_EXAMPLES_ACR}/SimpleResultCodes.cpp
    )

set (MUD_SRCS
    ${XSTX_EXAMPLES_MUD}/base/TimeVal.cpp
    )

set (SRCS
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/AudioModule.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/AudioRenderer.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/VideoModule.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/VideoRenderer.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/AppStreamWrapper.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/ffmpeg_decoder/AvHelper.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/ffmpeg_decoder/H264ToYuv.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/opus_decoder/OpusDecoder.cpp"
    "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/portaudio_renderer/PortAudioRenderer.cpp"
    )

if (WIN32)

    add_definitions(-D_AFXDLL)
    set(CMAKE_MFC_FLAG 2) # use Shared MFC dll

    # Generate PDB info
    set (CMAKE_CXX_FLAGS "/Zi ${CMAKE_CXX_FLAGS}")
    set (CMAKE_C_FLAGS "/Zi ${CMAKE_C_FLAGS}")


    set (CMAKE_CXX_FLAGS "/wd4005 /wd4996 /wd4099 ${CMAKE_CXX_FLAGS}")
    set (CMAKE_C_FLAGS "/wd4005 /wd4996 /wd4099 ${CMAKE_C_FLAGS}")

    include_directories ("${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows")
    include_directories ("${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/directX_renderer")

    # direct x include and link
    if(EXISTS "C:/Program Files (x86)/Microsoft DirectX SDK (August 2009)")
        set (DIRECTX_SDK_ROOT "C:/Program Files (x86)/Microsoft DirectX SDK (August 2009)" CACHE STRING  "Default location of the Direct X SDK")
    elseif (EXISTS "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
        set (DIRECTX_SDK_ROOT "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" CACHE STRING  "Default location of the Direct X SDK")
    else ()
        message(FATAL_ERROR "DirectX SDK is required to compile this client in windows! (August 2009 or June 2010)")
    endif (EXISTS "C:/Program Files (x86)/Microsoft DirectX SDK (August 2009)")
    include_directories ("${DIRECTX_SDK_ROOT}/Include")

    set (MUD_SRCS
        ${XSTX_EXAMPLES_MUD}/base/windows/WindowsTimeVal.cpp
        ${XSTX_EXAMPLES_MUD}/threading/windows/WindowsSimpleLock.cpp
        ${XSTX_EXAMPLES_MUD}/threading/windows/WindowsThread.cpp
        ${XSTX_EXAMPLES_MUD}/threading/windows/WindowsThreadUtil.cpp
        ${XSTX_EXAMPLES_MUD}/threading/windows/WindowsWaitableLock.cpp
        ${MUD_SRCS}
        )

    set (SRCS
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/AudioPipeline.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/VideoPipeline.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/platformBindings.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/AppStreamClient.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/directX_renderer/DirectXRenderer.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/directX_renderer/DumpFrameTest.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/directX_renderer/GDICapture.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/ClientWindow.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/DES/AppStreamClientFileInput.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/DES/EntitlementRetrieverWindows.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/DES/WinEntitlementPromptDlg.cpp"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/AppStreamClient.rc"
        "${STX_EXAMPLE_CLIENTS_SOURCE_DIR}/windows/InputHandling/WindowInputCapture.cpp"
        ${ACR_SRCS}
        ${MUD_SRCS}
        ${SRCS}
        )
endif (WIN32)

add_executable(AppStreamClient ${SRCS})


target_link_libraries (AppStreamClient XStxClientLibraryShared)
target_link_libraries (AppStreamClient avformat avcodec swscale avutil)
target_link_libraries (AppStreamClient celt silk_common silk_fixed silk_float opus)
target_link_libraries (AppStreamClient portaudio_static_${Architecture})

if (WIN32)
    target_link_libraries (AppStreamClient "${DIRECTX_SDK_ROOT}/Lib/${Architecture}/d3d9.lib" "${DIRECTX_SDK_ROOT}/Lib/${Architecture}/d3dx9.lib")
    target_link_libraries (AppStreamClient winmm winhttp)

    # Generate MAP files
    set_target_properties(AppStreamClient PROPERTIES LINK_FLAGS "/DEBUG /MAP")
endif (WIN32)


install (TARGETS AppStreamClient DESTINATION "${CMAKE_INSTALL_PREFIX}/")
install (FILES "${PROJECT_BINARY_DIR}/AppStreamClient.pdb" DESTINATION "${CMAKE_INSTALL_PREFIX}/")
install (FILES "${PROJECT_BINARY_DIR}/AppStreamClient.map" DESTINATION "${CMAKE_INSTALL_PREFIX}/")


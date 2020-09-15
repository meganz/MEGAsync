
cmake_minimum_required(VERSION 3.10)

set(MEGA_PROJECT_NAME "MEGAupdateGenerator-" CACHE STRING "Project name, SDK will declare and append 32/64")
set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "Verbose output")

PROJECT(${MEGA_PROJECT_NAME})

set (UNCHECKED_ITERATORS 0 CACHE STRING "")

# This needs static CRT linkage:
set (MEGA_LINK_DYNAMIC_CRT 0 CACHE STRING "")

set(RepoDir "${CMAKE_CURRENT_LIST_DIR}/../.."  CACHE STRING "")
set(SDKDir "${RepoDir}/src/MEGASync/mega")
set(MEGAupdateGeneratorDir "${RepoDir}/src/MEGAUpdateGenerator")

### Use prebuild 3rdparties
#set (USE_PREBUILT_3RDPARTY 1 CACHE STRING "")
#set(prebuilt_dir "${SDKDir}/bindings/qt/3rdparty")
##specify where 3rd party libraries are available
#set(Mega3rdPartyDir "${prebuilt_dir}"  CACHE STRING "")

# Use vcpkg 3rdparties
set (USE_THIRDPARTY_FROM_VCPKG 1 CACHE STRING "")
set(Mega3rdPartyDir "C:/path/to/vcpkg/parent/folder"  CACHE STRING "")

set (USE_FREEIMAGE 0 CACHE STRING "") # thumbnail generation etc done with QT, so no need for freeimage

# this line points to the MEGA SDK repo that you want to build MEGAsync against
include(${SDKDir}/contrib/cmake/CMakeLists.txt)

# avoid building other targets:
get_property(ALL_BUILDSYSTEM_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach (TARG IN LISTS ALL_BUILDSYSTEM_TARGETS)
    message(STATUS "Excluding from build:  ${TARG}")
    set_target_properties(${TARG} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endforeach(TARG)

add_definitions( -DSHOW_LOGS -D_CRT_SECURE_NO_WARNINGS -DPSAPI_VERSION=1 )
add_definitions( -DUSE_CRYPTOPP )

if(WIN32)
    add_definitions( -DUNICODE -D_UNICODE )  # needed for visual studio projects to use the unicode runtime libraries
endif()

if (CMAKE_HOST_WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127")  # 4127: conditional expression is constant  (occurs in QT headers)
endif()

set (UPDATEGENERATOR_FILES
    ${MEGAupdateGeneratorDir}/MEGAUpdateGenerator.cpp
    ${SDKDir}/src/crypto/cryptopp.cpp
    ${SDKDir}/src/base64.cpp
    ${SDKDir}/src/logging.cpp
)

ImportStdVcpkgLibrary(cryptopp-staticcrt cryptopp-staticcrt cryptopp-staticcrt libcryptopp libcryptopp)

if(CMAKE_HOST_APPLE)
    add_executable(MEGAUpdateGenerator MACOSX_BUNDLE ${MAC_RESOURCES} ${UPDATEGENERATOR_FILES} )
    target_link_libraries(MEGAUpdateGenerator cryptopp-staticcrt)
    target_include_directories(MEGAUpdateGenerator PRIVATE ${SDKDir}/include/mega/posix)
else(CMAKE_HOST_APPLE)
    add_executable(MEGAUpdateGenerator ${UPDATEGENERATOR_FILES} )
    if(CMAKE_HOST_WIN32)
        target_link_libraries(MEGAUpdateGenerator cryptopp-staticcrt ws2_32 Urlmon.lib Shlwapi.lib)
        target_include_directories(MEGAUpdateGenerator PRIVATE ${SDKDir}/include/mega/win32)
        set_property(TARGET MEGAUpdateGenerator PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else(CMAKE_HOST_WIN32)
        target_link_libraries(MEGAUpdateGenerator cryptopp-staticcrt)
        target_include_directories(MEGAUpdateGenerator PRIVATE ${SDKDir}/include/mega/posix)
    endif(CMAKE_HOST_WIN32)
endif(CMAKE_HOST_APPLE)

target_include_directories(MEGAUpdateGenerator PRIVATE ${SDKDir}/include)



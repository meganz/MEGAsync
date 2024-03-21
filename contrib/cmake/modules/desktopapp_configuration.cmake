#
# Golbal CMake configuration for the MEGAchat project
#

# Build the project with C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_HOST_APPLE)

    # Minimum deployment target differs if we are building for intel or arm64 targets
    # CMAKE_SYSTEM_PROCESSOR and CMAKE_HOST_SYSTEM_PROCESSOR are only available after project()
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE HOST_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Setup CMAKE_OSX_DEPLOYMENT_TARGET before project()
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64" OR (NOT CMAKE_OSX_ARCHITECTURES AND HOST_ARCHITECTURE STREQUAL "arm64"))
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.1" CACHE STRING "Minimum OS X deployment version")
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
    endif()

    message(STATUS "Minimum OS X deployment version is set to ${CMAKE_OSX_DEPLOYMENT_TARGET}")

    unset(HOST_ARCHITECTURE)

endif()

# if (WIN32)
#     # Build the project with C++17 for Windows
#     set(CMAKE_CXX_STANDARD 17)
#     set(CMAKE_CXX_STANDARD_REQUIRED ON)
#     if (MSVC)
#         # https://gitlab.kitware.com/cmake/cmake/-/issues/18837
#         add_compile_options(/Zc:__cplusplus) # Enable updated __cplusplus macro
#     endif()

#     # Enable build with multiple processes.
#     add_compile_options(/MP)

#     # Create a separated PDB file with debug symbols.
#     add_compile_options($<$<CONFIG:Release>:/Zi>)

# else()
#     # Build the project with C++11
#     set(CMAKE_CXX_STANDARD 11)
#     set(CMAKE_CXX_STANDARD_REQUIRED ON)

#     include(CheckIncludeFile)
#     include(CheckFunctionExists)
#     check_include_file(inttypes.h HAVE_INTTYPES_H)
#     check_include_file(dirent.h HAVE_DIRENT_H)
#     check_include_file(glob.h HAVE_GLOB_H)
#     check_function_exists(aio_write, HAVE_AIO_RT)
# endif()


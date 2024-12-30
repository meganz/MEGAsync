#
# Initial CMake configuration for the MEGA Desktop app project
#

if(CMAKE_HOST_APPLE)
    # Required flags required to generate correct stack traces on a crash
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

    # Minimum deployment target differs if we are building for intel or arm64 targets
    # CMAKE_SYSTEM_PROCESSOR and CMAKE_HOST_SYSTEM_PROCESSOR are only available after project()
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE HOST_ARCHITECTURE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Setup CMAKE_OSX_DEPLOYMENT_TARGET before project()
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR (NOT CMAKE_OSX_ARCHITECTURES AND HOST_ARCHITECTURE STREQUAL "arm64"))
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.1" CACHE STRING "Minimum OS X deployment version")
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    endif()

    message(STATUS "Minimum OS X deployment version is set to ${CMAKE_OSX_DEPLOYMENT_TARGET}")

    unset(HOST_ARCHITECTURE)

endif()


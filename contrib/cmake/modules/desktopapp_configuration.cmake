#
# Global CMake configuration for the MEGA Desktop app project
#

# Build the project with C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<BOOL:${WIN32}>:NTDDI_VERSION=NTDDI_WIN7> # Supported windows versions: 7 and beyond
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0601>      # 0x0601 == Windows 7
    $<$<BOOL:${WIN32}>:PSAPI_VERSION=1>          # PS API version used in Windows 7                  # PS API version used in Windows 7
)

if (MSVC)
    # https://gitlab.kitware.com/cmake/cmake/-/issues/18837
    add_compile_options(/Zc:__cplusplus) # Enable updated __cplusplus macro

    # Enable build with multiple processes.
    add_compile_options(/MP)

    # std::list::erase is very slow with _ITERATOR_DEBUG_LEVEL=2, which is default setting
    # in MSVC Debug compilation mode.
    add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)
endif()

if(UNIX AND NOT APPLE)
    # Set rpath and location for dirs accordingly:
    # If CMAKE_INSTALL_PREFIX is set (not default), it will set rpath to such prefix plus /opt/....
    # If CMAKE_INSTALL_PREFIX is not set (default), it will set rpath to /opt/....
    # Note: using cmake --install --prefix /some/prefix will keep the RPATH as configured above.
    #       Used for building packages: in which install dir is a path construction folder that will not be there in packages

    set(CMAKE_INSTALL_LIBDIR "opt/megasync/lib") # override default "lib" from GNUInstallDirs
    set(CMAKE_INSTALL_BINDIR "usr/bin") # override default "bin" from GNUInstallDirs

    # Override CMAKE_INSTALL_PREFIX
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) # In consecutive runs it will always be undef/false
        message(STATUS "Overriding default CMAKE_INSTALL_PREFIX")
        set(CMAKE_INSTALL_PREFIX "" CACHE PATH "" FORCE) # Set value in cache for consecutive runs
    endif()

    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

    message(STATUS "Current installation path for ${PROJECT_NAME} project: \"${CMAKE_INSTALL_PREFIX}\"")

endif()

#
# Global CMake configuration for the MEGA Desktop app project
#

# Build the project with C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # https://gitlab.kitware.com/cmake/cmake/-/issues/18837
    add_compile_options(/Zc:__cplusplus) # Enable updated __cplusplus macro

    # Enable build with multiple processes.
    add_compile_options(/MP)

    # Create a separated PDB file with debug symbols.
    add_compile_options($<$<CONFIG:Release>:/Zi>)
endif()


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
endif()


#
# MEGA Update
# Binary to perform updates in a separate process registered in target systems.
#

add_executable(MEGAupdater)

set(UPDATER_HEADERS
    MacUtils.h
    Preferences.h
    UpdateTask.h
)

set(UPDATER_SOURCES
    MacUtils.mm
    MegaUpdater.cpp
    UpdateTask.cpp
)


target_sources(MEGAupdater
    PRIVATE
    ${UPDATER_HEADERS}
    ${UPDATER_SOURCES}
)


# Load and link needed libraries for the CHATlib target
find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(MEGAupdater PRIVATE cryptopp::cryptopp)

if(WIN32)
    # target_link_libraries(MEGAupdater PRIVATE
    #     ws2_32 winhttp Shlwapi Secur32.lib crypt32.lib
    #     $<$<BOOL:${USE_CURL}>:Wldap32.lib>
    #     $<$<BOOL:${USE_LIBUV}>:Kernel32.lib Iphlpapi.lib Userenv.lib Psapi.lib>
    #     $<$<BOOL:${USE_FFMPEG}>:Mfplat.lib mfuuid.lib strmiids.lib>
    #     $<$<BOOL:${ENABLE_DRIVE_NOTIFICATIONS}>:wbemuuid>
    # )
else()
    if(APPLE)
        target_link_libraries(MEGAupdater PRIVATE
            "-framework CoreServices"
            "-framework Cocoa"
        )
    endif()
endif()


## Adjust compilation flags for warnings and errors ##
# target_platform_compile_options(
#     TARGET MEGAupdater
#     UNIX $<$<CONFIG:Debug>:-ggdb3> -Wall -Wextra -Wconversion -Wno-unused-parameter
# )

if(ENABLE_DESKTOP_APP_WERROR)
    target_platform_compile_options(
        TARGET MEGAupdater
        UNIX  $<$<CONFIG:Debug>: -Werror
                                 -Wno-error=deprecated-declarations> # Kept as a warning, do not promote to error.
    )
endif()

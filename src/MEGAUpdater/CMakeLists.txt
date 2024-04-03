#
# MEGA Updater
# Binary to perform updates in a separate process registered in target systems.
#

if (WIN32)
    add_executable(MEGAupdater WIN32)
elseif (APPLE)
    add_executable(MEGAupdater MACOSX_BUNDLE)
else()
endif()

set(UPDATER_HEADERS
    Preferences.h
    UpdateTask.h
)

set(UPDATER_SOURCES
    MegaUpdater.cpp
    UpdateTask.cpp
)

if (APPLE)
    list(APPEND UPDATER_SOURCES
        MacUtils.mm
    )
endif()

target_sources(MEGAupdater
    PRIVATE
    ${UPDATER_HEADERS}
    ${UPDATER_SOURCES}
)

# Load and link needed libraries for the MEGAupdater target
find_package(cryptopp CONFIG REQUIRED)

if (WIN32)
    # target_link_libraries(MEGAupdater PRIVATE
    #     ws2_32 winhttp Shlwapi Secur32.lib crypt32.lib
    #     $<$<BOOL:${USE_CURL}>:Wldap32.lib>
    #     $<$<BOOL:${USE_LIBUV}>:Kernel32.lib Iphlpapi.lib Userenv.lib Psapi.lib>
    #     $<$<BOOL:${USE_FFMPEG}>:Mfplat.lib mfuuid.lib strmiids.lib>
    #     $<$<BOOL:${ENABLE_DRIVE_NOTIFICATIONS}>:wbemuuid>
    # )
    target_compile_definitions(MEGAupdater
        PUBLIC
        UNICODE
    )

    target_link_libraries(MEGAupdater PRIVATE
        cryptopp::cryptopp
        urlmon Shlwapi
    )

elseif (APPLE)
        target_link_libraries(MEGAupdater PRIVATE
            cryptopp::cryptopp
            "-framework CoreServices"
            "-framework Cocoa"
        )
else()

endif()

if (ENABLE_DESKTOP_APP_WERROR)
    target_platform_compile_options(
        TARGET MEGAupdater
        UNIX  $<$<CONFIG:Debug>: -Werror
                                 -Wno-error=deprecated-declarations> # Kept as a warning, do not promote to error.
    )
endif()

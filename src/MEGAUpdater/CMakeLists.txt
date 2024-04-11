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

target_sources_conditional(MEGAupdater
   FLAG APPLE
   PRIVATE
   MacUtils.mm
)

target_sources(MEGAupdater
    PRIVATE
    ${UPDATER_HEADERS}
    ${UPDATER_SOURCES}
)

# Load and link needed libraries for the MEGAupdater target
find_package(cryptopp CONFIG REQUIRED)

if (WIN32)
    target_compile_definitions(MEGAupdater
        PUBLIC
        UNICODE
    )
endif()

target_link_libraries(MEGAupdater
    PRIVATE
    cryptopp::cryptopp
    $<$<BOOL:${WIN32}>:urlmon Shlwapi>
    $<$<BOOL:${APPLE}>:"-framework CoreServices">
    $<$<BOOL:${APPLE}>:"-framework Cocoa">
)

if (ENABLE_DESKTOP_APP_WERROR)
    target_platform_compile_options(
        TARGET MEGAupdater
        UNIX  $<$<CONFIG:Debug>: -Werror
                                 -Wno-error=deprecated-declarations> # Kept as a warning, do not promote to error.
    )
endif()

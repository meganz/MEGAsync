#
# MEGA Desktop app project specific options
#

if(WIN32)
    option(ENABLE_EXPLORER_EXT "desc" ON)
elseif(APPLE)
    option(ENABLE_FINDER_EXT "desc" ON)
else()
    #LINUX
    option(ENABLE_LINUX_EXT "desc" OFF) #Need to port nautilus and thunar to cmake
endif()


option(ENABLE_DESKTOP_UPDATER "Enable desktop updater tool build" ON)
option(ENABLE_DESKTOP_APP "Enable desktop app build" ON)
option(ENABLE_DESKTOP_UPDATE_GEN "Enable desktop update generator tool" ON)
option(ENABLE_DESKTOP_APP_WERROR "Enable warnings as errors" OFF)

# MEGAsdk options
# Configure MEGAsdk specific options for MEGAchat and then load the rest of MEGAsdk configuration
if (ENABLE_DESKTOP_APP)
    set(USE_LIBUV ON) # Used by the Qt Desktop App: Includes the library and turns on internal web and ftp server functionality in the SDK.
    set(ENABLE_LOG_PERFORMANCE ON)
    set(ENABLE_QT_BINDINGS ON)

    if (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "10.15")
        set(ENABLE_ISOLATED_GFX OFF) #Disable isolated GFX processing on x86_64 due to problems with macOS 10.13
    endif()

endif()

include(sdklib_options)


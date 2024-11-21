#
# MEGA Desktop app project specific options
#

if(WIN32)
    option(ENABLE_EXPLORER_EXT "desc" ON)
    option(ENABLE_EXPLORER_EXT_SPARSE "Enable Win11 sparse package" ON)
    option(ENABLE_DESKTOP_UPDATER "Enable desktop updater tool build" ON)
elseif(APPLE)
    option(ENABLE_FINDER_EXT "desc" ON)
    option(ENABLE_DESKTOP_UPDATER "Enable desktop updater tool build" ON)
else()
    #LINUX
    option(ENABLE_LINUX_EXT "desc" OFF) #Need to port nautilus and thunar to cmake
    option(ENABLE_DESKTOP_UPDATER "Enable desktop updater tool build" OFF)
    option(DEPLOY_QT_LIBRARIES "Deploy Qt libraries with MEGAsync during the install phase" OFF)
endif()

option(ENABLE_DESKTOP_APP "Enable desktop app build" ON)
option(ENABLE_DESKTOP_UPDATE_GEN "Enable desktop update generator tool" ON)

option(ENABLE_DESKTOP_APP_WERROR "Enable warnings as errors" ON)
option(ENABLE_DESIGN_TOKENS_IMPORTER "Enable design tokens importer tool" OFF)
option(ENABLE_DESKTOP_APP_TESTS "Enable Desktop app Automated tests" ON)

# MEGAsdk options
# Configure MEGAsdk specific options for MEGAchat and then load the rest of MEGAsdk configuration
if (ENABLE_DESKTOP_APP)
    set(USE_LIBUV ON) # Used by the Qt Desktop App: Includes the library and turns on internal web and ftp server functionality in the SDK.
    set(ENABLE_LOG_PERFORMANCE ON)
    set(ENABLE_QT_BINDINGS ON)
    set(ENABLE_ISOLATED_GFX ON)
endif()

include(sdklib_options)


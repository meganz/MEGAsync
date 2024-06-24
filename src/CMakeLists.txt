## Load targets

# Load MEGA Desktop app project

if(ENABLE_DESKTOP_APP OR ENABLE_DESKTOP_UPDATE_GEN)
    add_subdirectory(MEGASync/mega)
endif()

if(ENABLE_DESKTOP_APP)
    add_subdirectory(MEGASync)
endif()

if(ENABLE_DESKTOP_UPDATE_GEN)
    add_subdirectory(MEGAUpdateGenerator)
endif()

if(ENABLE_DESKTOP_UPDATER)
    add_subdirectory(MEGAUpdater)
endif()

if(ENABLE_EXPLORER_EXT)
    add_subdirectory(MEGAShellExt)
endif()

if(ENABLE_FINDER_EXT)
    # add_subdirectory(MEGAShellExtFinder)
endif()

if(ENABLE_LINUX_EXT)
    # add_subdirectory(MEGAShellExtDolphin)
    # add_subdirectory(MEGAShellExtNautilus)
    # add_subdirectory(MEGAShellExtThunar)
    # add_subdirectory(MEGAShellExtNemo)
endif()

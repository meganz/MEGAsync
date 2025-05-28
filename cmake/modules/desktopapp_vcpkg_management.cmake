macro(process_desktop_app_vcpkg_libraries)

    if (USE_BREAKPAD)
        list(APPEND VCPKG_MANIFEST_FEATURES "use-breakpad")
    endif()

    if (ENABLE_DESKTOP_APP_TESTS)
        list(APPEND VCPKG_MANIFEST_FEATURES "desktop-tests")
    endif()

endmacro()

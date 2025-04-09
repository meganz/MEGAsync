macro(process_desktop_app_vcpkg_libraries)

    if (USE_BREAKPAD)
            list(APPEND VCPKG_MANIFEST_FEATURES "use-breakpad")
        endif()

endmacro()

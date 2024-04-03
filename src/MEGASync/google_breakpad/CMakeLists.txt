
add_library(google_breakpad)
add_library(MEGA::google_breakpad ALIAS google_breakpad)

set_property(TARGET google_breakpad PROPERTY CXX_STANDARD 11)

if (WIN32)
    set(BREAKPAD_SOURCES
        client/windows/handler/exception_handler.cc
        client/windows/crash_generation/crash_generation_client.cc
        common/windows/string_utils.cc
        common/windows/guid_string.cc
    )

    # Include main path to locate Version.h
    target_include_directories(google_breakpad PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/..
    )

elseif (APPLE)
    set(BREAKPAD_SOURCES
        client/mac/handler/exception_handler.cc
        client/mac/crash_generation/crash_generation_client.cc
        client/mac/crash_generation/crash_generation_server.cc
        client/mac/handler/minidump_generator.cc
        client/mac/handler/dynamic_images.cc
        client/mac/handler/breakpad_nlist_64.cc
        client/minidump_file_writer.cc
        common/mac/macho_id.cc
        common/mac/macho_walker.cc
        common/mac/macho_utilities.cc
        common/mac/string_utilities.cc
        common/mac/file_id.cc
        common/mac/bootstrap_compat.cc
        common/md5.cc
        common/string_conversion.cc
        common/linux/linux_libc_support.cc
        common/convert_UTF.c
        common/mac/MachIPC.mm
    )
else()

endif()

target_sources(google_breakpad
    PRIVATE
    ${BREAKPAD_SOURCES}
)

target_include_directories(google_breakpad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

if (WIN32)
    find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
    target_link_libraries(google_breakpad
        PRIVATE
        Qt5::Widgets
        Qt5::Core
        MEGA::SDKlib
    )
    target_compile_definitions(google_breakpad
        PUBLIC
        UNICODE
    )
endif()

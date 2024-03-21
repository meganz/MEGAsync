
add_library(google_breakpad)
add_library(MEGA::google_breakpad ALIAS google_breakpad)

set_property(TARGET google_breakpad PROPERTY CXX_STANDARD 11)

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

target_sources(google_breakpad
    PRIVATE
    ${BREAKPAD_SOURCES}
)

target_include_directories(google_breakpad PRIVATE ${CMAKE_CURRENT_LIST_DIR})

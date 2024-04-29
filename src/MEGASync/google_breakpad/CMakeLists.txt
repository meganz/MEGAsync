
add_library(google_breakpad)
add_library(MEGA::google_breakpad ALIAS google_breakpad)

set_property(TARGET google_breakpad PROPERTY CXX_STANDARD 11)

target_sources_conditional(google_breakpad
   FLAG WIN32
   PRIVATE
   client/windows/handler/exception_handler.cc
   client/windows/crash_generation/crash_generation_client.cc
   common/windows/string_utils.cc
   common/windows/guid_string.cc
)

target_sources_conditional(google_breakpad
   FLAG UNIX
   PRIVATE
   common/convert_UTF.c
   common/linux/linux_libc_support.cc
   common/string_conversion.cc
   client/minidump_file_writer.cc
)

target_sources_conditional(google_breakpad
   FLAG UNIX AND NOT APPLE
   PRIVATE
   client/linux/crash_generation/crash_generation_client.cc
   client/linux/handler/exception_handler.cc
   client/linux/handler/minidump_descriptor.cc
   client/linux/minidump_writer/minidump_writer.cc
   client/linux/minidump_writer/linux_dumper.cc
   client/linux/minidump_writer/linux_ptrace_dumper.cc
   client/linux/log/log.cc
   common/linux/file_id.cc
   common/linux/memory_mapped_file.cc
   common/linux/safe_readlink.cc
   common/linux/guid_creator.cc
   common/linux/elfutils.cc

)

target_sources_conditional(google_breakpad
   FLAG APPLE
   PRIVATE
   client/mac/handler/exception_handler.cc
   client/mac/crash_generation/crash_generation_client.cc
   client/mac/crash_generation/crash_generation_server.cc
   client/mac/handler/minidump_generator.cc
   client/mac/handler/dynamic_images.cc
   client/mac/handler/breakpad_nlist_64.cc
   common/mac/macho_id.cc
   common/mac/macho_walker.cc
   common/mac/macho_utilities.cc
   common/mac/string_utilities.cc
   common/mac/file_id.cc
   common/mac/bootstrap_compat.cc
   common/md5.cc
   common/mac/MachIPC.mm
)

# Include main path to locate Version.h
target_include_directories(google_breakpad
    PUBLIC
    "$<$<BOOL:${WIN32}>:${CMAKE_CURRENT_LIST_DIR}/..>"
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


macx {
  SOURCES += $$PWD/client/mac/handler/exception_handler.cc
  SOURCES += $$PWD/client/mac/crash_generation/crash_generation_client.cc
  SOURCES += $$PWD/client/mac/crash_generation/crash_generation_server.cc
  SOURCES += $$PWD/client/mac/handler/minidump_generator.cc
  SOURCES += $$PWD/client/mac/handler/dynamic_images.cc
  SOURCES += $$PWD/client/mac/handler/breakpad_nlist_64.cc
  SOURCES += $$PWD/client/minidump_file_writer.cc
  SOURCES += $$PWD/common/mac/macho_id.cc
  SOURCES += $$PWD/common/mac/macho_walker.cc
  SOURCES += $$PWD/common/mac/macho_utilities.cc
  SOURCES += $$PWD/common/mac/string_utilities.cc
  SOURCES += $$PWD/common/mac/file_id.cc
  SOURCES += $$PWD/common/mac/MachIPC.mm
  SOURCES += $$PWD/common/mac/bootstrap_compat.cc
  SOURCES += $$PWD/common/md5.cc
  SOURCES += $$PWD/common/string_conversion.cc
  SOURCES += $$PWD/common/linux/linux_libc_support.cc
  SOURCES += $$PWD/common/convert_UTF.c
  LIBS += /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
  LIBS += /System/Library/Frameworks/CoreServices.framework/Versions/A/CoreServices

  QMAKE_CXXFLAGS+=-g
}

unix:!macx {
  SOURCES += $$PWD/client/linux/crash_generation/crash_generation_client.cc
  SOURCES += $$PWD/client/linux/handler/exception_handler.cc
  SOURCES += $$PWD/client/linux/handler/minidump_descriptor.cc
  SOURCES += $$PWD/client/linux/minidump_writer/minidump_writer.cc
  SOURCES += $$PWD/client/linux/minidump_writer/linux_dumper.cc
  SOURCES += $$PWD/client/linux/minidump_writer/linux_ptrace_dumper.cc
  SOURCES += $$PWD/client/linux/log/log.cc
  SOURCES += $$PWD/client/minidump_file_writer.cc
  SOURCES += $$PWD/common/linux/linux_libc_support.cc
  SOURCES += $$PWD/common/linux/file_id.cc
  SOURCES += $$PWD/common/linux/memory_mapped_file.cc
  SOURCES += $$PWD/common/linux/safe_readlink.cc
  SOURCES += $$PWD/common/linux/guid_creator.cc
  SOURCES += $$PWD/common/linux/elfutils.cc
  SOURCES += $$PWD/common/string_conversion.cc
  SOURCES += $$PWD/common/convert_UTF.c

  QMAKE_CXXFLAGS+=-g
}

win32 {
  SOURCES += $$PWD/client/windows/handler/exception_handler.cc
  SOURCES += $$PWD/common/windows/string_utils.cc
  SOURCES += $$PWD/common/windows/guid_string.cc
  SOURCES += $$PWD/client/windows/crash_generation/crash_generation_client.cc
}

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
INCLUDEPATH += $$PWD

#debug {
    DEFINES += CREATE_COMPATIBLE_MINIDUMPS
#}

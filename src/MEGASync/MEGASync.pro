#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T12:41:38
#
#-------------------------------------------------

win32:THIRDPARTY_VCPKG_BASE_PATH = $$PWD/../../../3rdParty-v140
win32:contains(QMAKE_TARGET.arch, x86_64):VCPKG_TRIPLET = x64-windows-mega
win32:!contains(QMAKE_TARGET.arch, x86_64):VCPKG_TRIPLET = x86-windows-mega

message("THIRDPARTY_VCPKG_BASE_PATH: $$THIRDPARTY_VCPKG_BASE_PATH")
message("VCPKG_TRIPLET: $$VCPKG_TRIPLET")


DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

debug_and_release {
    CONFIG -= debug_and_release
    CONFIG += debug_and_release
}
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    BUILD_ARCH=$${QT_ARCH}
} else {
    BUILD_ARCH=$${QMAKE_HOST.arch}
}

unix:!macx {
    QT += svg
    TARGET = megasync

#    Uncomment the following if "make install" doesn't copy megasync in /usr/bin directory
#    isEmpty(PREFIX) {
#        PREFIX = /usr
#    }
#    target.path = $$PREFIX/bin
#    INSTALLS += target
}
else {
    TARGET = MEGAsync
}

TEMPLATE = app

#DEFINES += LOG_TO_LOGGER
#DEFINES += LOG_TO_FILE
#DEFINES += LOG_TO_STDOUT
DEFINES += ENABLE_LOG_PERFORMANCE

debug {
    CONFIG += console
    DEFINES += CREATE_COMPATIBLE_MINIDUMPS
    DEFINES += LOG_TO_STDOUT
#   DEFINES += LOG_TO_FILE
    DEFINES += LOG_TO_LOGGER
}

CONFIG += USE_LIBUV
CONFIG += USE_MEGAAPI
CONFIG += USE_MEDIAINFO
CONFIG += USE_LIBRAW
CONFIG += USE_FFMPEG

macx {
CONFIG += USE_PDFIUM
}
else:win32 {
CONFIG += USE_PDFIUM
DEFINES += NOMINMAX
}
else:contains(BUILD_ARCH, x86_64) { #Notice this might not work for clang!
CONFIG += USE_PDFIUM
}

unix:!macx {
        exists(/usr/include/ffmpeg-mega)|exists(mega/bindings/qt/3rdparty/include/ffmpeg)|packagesExist(ffmpeg)|packagesExist(libavcodec) {
            CONFIG += USE_FFMPEG
        }
}
else {
    CONFIG += USE_FFMPEG
}

include(gui/gui.pri)
include(mega/bindings/qt/sdk.pri)
include(control/control.pri)
include(platform/platform.pri)
include(google_breakpad/google_breakpad.pri)
include(qtlockedfile/qtlockedfile.pri)

unix:!macx {
    GCC_VERSION = $$system("g++ -dumpversion")
    lessThan(GCC_VERSION, 5) {
        LIBS -= -lstdc++fs
    }
}

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += MegaApplication.cpp
HEADERS += MegaApplication.h

HEADERS += $$PWD/spdlog/common-inl.h \
    $$PWD/spdlog/spdlog.h \
    $$PWD/spdlog/version.h \
    $$PWD/spdlog/async_logger-inl.h \
    $$PWD/spdlog/details/fmt_helper.h \
    $$PWD/spdlog/details/backtracer-inl.h \
    $$PWD/spdlog/details/mpmc_blocking_q.h \
    $$PWD/spdlog/details/registry-inl.h \
    $$PWD/spdlog/details/synchronous_factory.h \
    $$PWD/spdlog/details/console_globals.h \
    $$PWD/spdlog/details/thread_pool-inl.h \
    $$PWD/spdlog/details/os-inl.h \
    $$PWD/spdlog/details/circular_q.h \
    $$PWD/spdlog/details/periodic_worker.h \
    $$PWD/spdlog/details/null_mutex.h \
    $$PWD/spdlog/details/backtracer.h \
    $$PWD/spdlog/details/log_msg_buffer-inl.h \
    $$PWD/spdlog/details/log_msg_buffer.h \
    $$PWD/spdlog/details/os.h \
    $$PWD/spdlog/details/pattern_formatter.h \
    $$PWD/spdlog/details/file_helper.h \
    $$PWD/spdlog/details/log_msg-inl.h \
    $$PWD/spdlog/details/registry.h \
    $$PWD/spdlog/details/thread_pool.h \
    $$PWD/spdlog/details/pattern_formatter-inl.h \
    $$PWD/spdlog/details/file_helper-inl.h \
    $$PWD/spdlog/details/periodic_worker-inl.h \
    $$PWD/spdlog/details/log_msg.h \
    $$PWD/spdlog/fmt/ostr.h \
    $$PWD/spdlog/fmt/fmt.h \
    $$PWD/spdlog/fmt/bin_to_hex.h \
    $$PWD/spdlog/fmt/bundled/compile.h \
    $$PWD/spdlog/fmt/bundled/ranges.h \
    $$PWD/spdlog/fmt/bundled/format-inl.h \
    $$PWD/spdlog/fmt/bundled/format.h \
    $$PWD/spdlog/fmt/bundled/color.h \
    $$PWD/spdlog/fmt/bundled/core.h \
    $$PWD/spdlog/fmt/bundled/locale.h \
    $$PWD/spdlog/fmt/bundled/posix.h \
    $$PWD/spdlog/fmt/bundled/LICENSE.rst \
    $$PWD/spdlog/fmt/bundled/safe-duration-cast.h \
    $$PWD/spdlog/fmt/bundled/printf.h \
    $$PWD/spdlog/fmt/bundled/ostream.h \
    $$PWD/spdlog/fmt/bundled/chrono.h \
    $$PWD/spdlog/logger.h \
    $$PWD/spdlog/logger-inl.h \
    $$PWD/spdlog/async.h \
    $$PWD/spdlog/async_logger.h \
    $$PWD/spdlog/spdlog-inl.h \
    $$PWD/spdlog/tweakme.h \
    $$PWD/spdlog/formatter.h \
    $$PWD/spdlog/sinks/msvc_sink.h \
    $$PWD/spdlog/sinks/stdout_sinks.h \
    $$PWD/spdlog/sinks/sink-inl.h \
    $$PWD/spdlog/sinks/base_sink.h \
    $$PWD/spdlog/sinks/base_sink-inl.h \
    $$PWD/spdlog/sinks/dist_sink.h \
    $$PWD/spdlog/sinks/basic_file_sink.h \
    $$PWD/spdlog/sinks/sink.h \
    $$PWD/spdlog/sinks/ostream_sink.h \
    $$PWD/spdlog/sinks/syslog_sink.h \
    $$PWD/spdlog/sinks/rotating_file_sink.h \
    $$PWD/spdlog/sinks/stdout_sinks-inl.h \
    $$PWD/spdlog/sinks/stdout_color_sinks-inl.h \
    $$PWD/spdlog/sinks/daily_file_sink.h \
    $$PWD/spdlog/sinks/ansicolor_sink.h \
    $$PWD/spdlog/sinks/wincolor_sink-inl.h \
    $$PWD/spdlog/sinks/systemd_sink.h \
    $$PWD/spdlog/sinks/dup_filter_sink.h \
    $$PWD/spdlog/sinks/stdout_color_sinks.h \
    $$PWD/spdlog/sinks/ansicolor_sink-inl.h \
    $$PWD/spdlog/sinks/null_sink.h \
    $$PWD/spdlog/sinks/basic_file_sink-inl.h \
    $$PWD/spdlog/sinks/wincolor_sink.h \
    $$PWD/spdlog/sinks/rotating_file_sink-inl.h \
    $$PWD/spdlog/sinks/android_sink.h \
    $$PWD/spdlog/common.h

TRANSLATIONS = \
    gui/translations/MEGASyncStrings_ar.ts \
    gui/translations/MEGASyncStrings_de.ts \
    gui/translations/MEGASyncStrings_en.ts \
    gui/translations/MEGASyncStrings_es.ts \
    gui/translations/MEGASyncStrings_fr.ts \
    gui/translations/MEGASyncStrings_id.ts \
    gui/translations/MEGASyncStrings_it.ts \
    gui/translations/MEGASyncStrings_ja.ts \
    gui/translations/MEGASyncStrings_ko.ts \
    gui/translations/MEGASyncStrings_nl.ts \
    gui/translations/MEGASyncStrings_pl.ts \
    gui/translations/MEGASyncStrings_pt_BR.ts \
    gui/translations/MEGASyncStrings_pt.ts \
    gui/translations/MEGASyncStrings_ro.ts \
    gui/translations/MEGASyncStrings_ru.ts \
    gui/translations/MEGASyncStrings_th.ts \
    gui/translations/MEGASyncStrings_tl.ts \
    gui/translations/MEGASyncStrings_uk.ts \
    gui/translations/MEGASyncStrings_vi.ts \
    gui/translations/MEGASyncStrings_zh_CN.ts \
    gui/translations/MEGASyncStrings_zh_TW.ts

CODECFORTR = UTF8

win32 {
    greaterThan(QT_MAJOR_VERSION, 4) {
        greaterThan(QT_MINOR_VERSION, 1) {
            QT += winextras
        }
    }

    RC_FILE = icon.rc
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
    QMAKE_LFLAGS_WINDOWS += /SUBSYSTEM:WINDOWS,5.01
    QMAKE_LFLAGS_CONSOLE += /SUBSYSTEM:CONSOLE,5.01
    DEFINES += PSAPI_VERSION=1
}


macx {
    QMAKE_CXXFLAGS += -DCRYPTOPP_DISABLE_ASM -D_DARWIN_C_SOURCE
    MAC_ICONS_RESOURCES.files += folder.icns
    MAC_ICONS_RESOURCES.files += folder_yosemite.icns
    MAC_ICONS_RESOURCES.files += appicon32.tiff
    MAC_ICONS_RESOURCES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += MAC_ICONS_RESOURCES
    ICON = app.icns

    QMAKE_INFO_PLIST = Info_MEGA.plist

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
    QMAKE_LFLAGS += -F /System/Library/Frameworks/Security.framework/
}


CONFIG(FULLREQUIREMENTS) {
DEFINES += REQUIRE_HAVE_FFMPEG
DEFINES += REQUIRE_HAVE_LIBUV
DEFINES += REQUIRE_HAVE_LIBRAW
DEFINES += REQUIRE_USE_MEDIAINFO

macx {
DEFINES += REQUIRE_HAVE_PDFIUM
}
else:win32 {
DEFINES += REQUIRE_HAVE_PDFIUM
}
else:contains(BUILD_ARCH, x86_64) { #Notice this might not work for clang!
DEFINES += REQUIRE_HAVE_PDFIUM
}

#DEFINES += REQUIRE_ENABLE_CHAT
#DEFINES += REQUIRE_ENABLE_BACKUPS
#DEFINES += REQUIRE_ENABLE_WEBRTC
#DEFINES += REQUIRE_ENABLE_EVT_TLS
#DEFINES += REQUIRE_USE_PCRE
}

CONFIG(debug) {
    OUTPATH=debug
}
CONFIG(release) {
    OUTPATH=release
}

win32 {
    QMAKE_POST_LINK = $$quote(mt.exe -nologo -manifest $$shell_path($$PWD/../../contrib/cmake/MEGAsync.exe.manifest) -outputresource:$$shell_path($${OUTPATH}/$${TARGET}.exe);1$$escape_expand(\n\t))
}

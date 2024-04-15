
set(DESKTOP_APP_LOCKED_FILE_HEADERS
    qtlockedfile/qtlockedfile.h
)

set(DESKTOP_APP_LOCKED_FILE_SOURCES
    qtlockedfile/qtlockedfile.cpp
)

target_sources_conditional(MEGAsync
   FLAG UNIX
   QT_AWARE
   PRIVATE
   qtlockedfile/qtlockedfile_unix.cpp
)

target_sources_conditional(MEGAsync
   FLAG WIN32
   QT_AWARE
   PRIVATE
   qtlockedfile/qtlockedfile_win.cpp
)

target_sources(MEGAsync
    PRIVATE
    ${DESKTOP_APP_LOCKED_FILE_HEADERS}
    ${DESKTOP_APP_LOCKED_FILE_SOURCES}
)

target_include_directories(MEGAsync PRIVATE ${CMAKE_CURRENT_LIST_DIR})

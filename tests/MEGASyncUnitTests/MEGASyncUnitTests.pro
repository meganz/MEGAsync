TARGET = MEGASyncUnitTests

CONFIG += qt console warn_on depend_includepath

CONFIG += c++14
CONFIG += with_tests

include(../../src/MEGASync/MEGASync.pro)
include(../3rdparty/catch/catch.pri)
include(../3rdparty/trompeloeil/trompeloeil.pri)
SOURCES += GuestWidgetTest.cpp \
           main.cpp

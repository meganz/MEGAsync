QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++14
CONFIG += with_tests

TEMPLATE = app
include(../../src/MEGASync/MEGASync.pro)
include(../3rdparty/catch/catch.pri)
include(../3rdparty/trompeloeil/trompeloeil.pri)
SOURCES += GuestWidgetTest.cpp \
           main.cpp

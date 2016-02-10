CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MEGACrashAnalyzer
TEMPLATE = app

HEADERS += \
    MainWindow.h

SOURCES += \
    MEGACrashAnalyzer.cpp \
    MainWindow.cpp

FORMS += \
    MainWindow.ui

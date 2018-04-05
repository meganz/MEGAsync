TEMPLATE = app
CONFIG -= qt
TARGET = MEGADeprecatedVersion
OBJECTIVE_SOURCES += MEGADeprecatedVersion.mm
LIBS += -framework Cocoa
ICON = app.icns
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
QMAKE_CXXFLAGS -= -stdlib=libc++
QMAKE_LFLAGS -= -stdlib=libc++
CONFIG -= c++11
QMAKE_CXXFLAGS += -g

QT       -= gui
QT       += network

TARGET = megasyncplugin
TEMPLATE = lib

SOURCES += megasync-plugin.cpp

HEADERS += megasync-plugin.h

# library
target.path = $$system(kde4-config --path module | cut -d ":" -f2)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

QMAKE_CXXFLAGS += -std=c++11


#greaterThan(QT_MAJOR_VERSION, 4): QT += KIOCore KIOFileWidgets KIOWidgets KNTLM

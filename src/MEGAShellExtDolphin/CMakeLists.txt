project(megasyncplugin)

cmake_minimum_required(VERSION 2.8.11)
FIND_PACKAGE( KDE4 REQUIRED )
INCLUDE( KDE4Defaults )

include_directories( ${QT_INCLUDES} ${KDE4_INCLUDES} ${LIBKONQ_INCLUDE_DIR} )

kde4_add_plugin(megasyncplugin megasync-plugin.cpp)
target_link_libraries(megasyncplugin ${KDE4_KIO_LIBS})

install(FILES megasync-plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS megasyncplugin DESTINATION ${PLUGIN_INSTALL_DIR})

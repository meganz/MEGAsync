project(megasync-plugin)

FIND_PACKAGE( KDE4 REQUIRED )
INCLUDE( KDE4Defaults )

include_directories( ${QT_INCLUDES} ${KDE4_INCLUDES} ${LIBKONQ_INCLUDE_DIR} )

kde4_add_plugin(megasync-plugin megasync-plugin.cpp)
target_link_libraries(megasync-plugin ${KDE4_KIO_LIBS})

install(FILES megasync-plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS megasync-plugin DESTINATION ${PLUGIN_INSTALL_DIR})

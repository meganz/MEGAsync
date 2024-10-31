# Guess the libraries path based on the Qt5_DIR path.
string(REGEX REPLACE "/lib/[^ ]*" "" DEPLOY_QT_LIBRARIES_PATH "${Qt5_DIR}")
if(NOT DEPLOY_QT_LIBRARIES_PATH)
    message(FATAL "Path to deploy the Qt libraries not found. Qt dir path used as base: ${Qt5_DIR}")
else()
    message(STATUS "Qt libraries to be deployed: \"${DEPLOY_QT_LIBRARIES_PATH}\"")
endif()

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/lib/"
    TYPE LIB # In CMAKE_INSTALL_LIBDIR directory
    FILES_MATCHING
    # Direct linkage
    PATTERN "libQt5X11Extras.so*"
    PATTERN "libQt5Svg.so*"
    PATTERN "libQt5Widgets.so*"
    PATTERN "libQt5Quick.so*"
    PATTERN "libQt5Gui.so*"
    PATTERN "libQt5Qml.so*"
    PATTERN "libQt5QmlModels.so*"
    PATTERN "libQt5Network.so*"
    PATTERN "libQt5Core.so*"
    PATTERN "libicui18n.so*"
    PATTERN "libicuuc.so*"
    PATTERN "libicudata.so*"
    # in lib directory
    PATTERN "libQt5QuickControls2.so*"
    PATTERN "libQt5QmlWorkerScript.so*"
    PATTERN "libQt5QuickTemplates2.so*"
    PATTERN "libQt5XcbQpa.so*"
    PATTERN "libQt5DBus.so*"
    PATTERN "libQt5RemoteObjects.so*"
    REGEX "/cmake|/metatypes|/pkgconfig" EXCLUDE
)

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/plugins/platforms/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../plugins/platforms"
    FILES_MATCHING
    PATTERN "libqxcb.so*"
    PATTERN "libqvnc.so*"
    PATTERN "libqoffscreen.so*"
    PATTERN "libqminimal.so*"
    PATTERN "libqlinuxfb.so*"
)

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/plugins/platforminputcontexts/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../plugins/platforminputcontexts"
    FILES_MATCHING
    PATTERN "libibusplatforminputcontextplugin.so*"
    PATTERN "libcomposeplatforminputcontextplugin.so*"
)

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/plugins/imageformats/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../plugins/imageformats"
    FILES_MATCHING
    PATTERN "libqwebp.so*"
    PATTERN "libqwbmp.so*"
    PATTERN "libqtiff.so*"
    PATTERN "libqtga.so*"
    PATTERN "libqsvg.so*"
    PATTERN "libqjpeg.so*"
    PATTERN "libqico.so*"
    PATTERN "libqicns.so*"
    PATTERN "libqgif.so*"
)

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/plugins/iconengines/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../plugins/iconengines"
    FILES_MATCHING
    PATTERN "libqsvgicon.so*"
)

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/plugins/bearer/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../plugins/bearer"
    FILES_MATCHING
    PATTERN "libqnmbearer.so*"
    PATTERN "libqgenericbearer.so*"
    PATTERN "libqconnmanbearer.so*"
)

install(DIRECTORY "${DEPLOY_QT_LIBRARIES_PATH}/plugins/xcbglintegrations/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../plugins/xcbglintegrations"
    FILES_MATCHING
    PATTERN "libqxcb-glx-integration.so*"
)

install(DIRECTORY
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQml"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick.2"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtGraphicalEffects"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../qml"
    REGEX ".debug" EXCLUDE
    REGEX ".qmlc" EXCLUDE
)

install(DIRECTORY
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Controls"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Controls.2"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Dialogs"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Extras"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Layouts"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/PrivateWidgets"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Templates.2"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/QtQuick/Window.2"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../qml/QtQuick"
    REGEX ".debug" EXCLUDE
    REGEX ".qmlc" EXCLUDE
)
install(DIRECTORY
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/Qt/labs/folderlistmodel"
    "${DEPLOY_QT_LIBRARIES_PATH}/qml/Qt/labs/settings"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/../qml/Qt/labs"
    REGEX ".debug" EXCLUDE
    REGEX ".qmlc" EXCLUDE
)

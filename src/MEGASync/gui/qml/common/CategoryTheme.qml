import QtQuick 2.15

Item {
    id: root

    property alias style: loader.item

    Loader {
        id: loader

        source: "qrc:/common/themes/"+themeManager.theme+"/Category.qml"
    }
}


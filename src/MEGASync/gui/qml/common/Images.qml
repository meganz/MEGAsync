pragma Singleton
import QtQuick 2.12

QtObject {
    readonly property bool designStudio: true

    readonly property string imagePath: designStudio ?
                                            "../../../../images/"
                                          : "qrc:/"
}

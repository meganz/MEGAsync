pragma Singleton
import QtQuick 2.12

QtObject {
    readonly property int width: 1000//640
    readonly property int height: 1000//480

    /* Edit this comment to add your custom font */
    readonly property font font: Qt.font({
                                             family: Qt.application.font.family,
                                             pixelSize: Qt.application.font.pixelSize
                                         })
    readonly property font largeFont: Qt.font({
                                                  family: Qt.application.font.family,
                                                  pixelSize: Qt.application.font.pixelSize * 1.6
                                              })

    readonly property color backgroundColor: "#c2c2c2"

    readonly property string fontFamily: "Inter"
    readonly property string fontStyleName: "normal"

}

pragma Singleton
import QtQuick 2.12

QtObject {
    property bool lightTheme: true

    readonly property int width: 640
    readonly property int height: 480

    /* Edit this comment to add your custom font */
    readonly property font font: Qt.font({
                                             family: Qt.application.font.family,
                                             pixelSize: Qt.application.font.pixelSize
                                         })
    readonly property font largeFont: Qt.font({
                                                  family: Qt.application.font.family,
                                                  pixelSize: Qt.application.font.pixelSize * 1.6
                                              })

    readonly property color backgroundColor: lightTheme ? "#F6F6F6" : "#282828"
    readonly property color alternateBackgroundColor: lightTheme ? "#FFFFFF" : "#18191A"
    readonly property color textColor: lightTheme ? "#04101E" : "#FAFAFB"
}

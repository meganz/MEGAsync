pragma Singleton
import QtQuick 2.15

QtObject {

    enum MessageType {
        NONE = 0,
        ERROR,
        WARNING,
        SUCCESS,
        INFO
    }

    readonly property string mega: "MEGA"

    readonly property int defaultWindowMargin: 36
    readonly property int focusBorderWidth: 4
    readonly property int focusAdjustment: -focusBorderWidth

}

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

    enum SyncType {
        NONE = 0,
        SYNC,
        SELECTIVE_SYNC,
        FULL_SYNC,
        BACKUP
    }

    readonly property string mega: "MEGA"

    readonly property int defaultWindowMargin: 36
    readonly property int defaultComponentSpacing: 24
    readonly property int focusBorderWidth: 4
    readonly property int focusAdjustment: -focusBorderWidth

}

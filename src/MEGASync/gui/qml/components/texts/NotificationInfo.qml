// System
import QtQuick 2.12

//Local
import Common 1.0

QtObject {

    enum Type {
        None = 0,
        Warning = 1,
        Error = 2
    }

    property int type: NotificationInfo.Type.None

    property color backgroundColor
    property color iconColor
    property color titleColor
    property color textColor

    property int margin: 12
    property int spacing: 8
    property int radius: 8

    property bool topBorderRect: false

    onTypeChanged: {
        switch(type) {
            case NotificationInfo.Type.None:
                break;
            case NotificationInfo.Type.Warning:
                if(icon.source.length === 0) {
                    icon.source = Images.alertTriangle;
                }
                iconColor = Styles.textWarning;
                titleColor = Styles.textWarning;
                textColor = Styles.textWarning;
                backgroundColor = Styles.notificationWarning;
                break;
            case NotificationInfo.Type.Error:
                if(icon.source.length === 0) {
                    icon.source = Images.xCircle;
                }
                iconColor = Styles.textError;
                titleColor = Styles.textError;
                textColor = Styles.textError;
                backgroundColor = Styles.notificationError;
                break;
            default:
                console.warn("NotificationInfo.Type -> " + type + " does not exist")
                break;
        }
    }

    property NotificationIcon icon: NotificationIcon {}
}

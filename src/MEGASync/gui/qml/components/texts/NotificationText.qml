// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// Local
import Components 1.0 as Custom
import Common 1.0

Rectangle {
    id: root

    enum Type {
        None = 0,
        Info,
        AuthenticationError
    }

    property int type: NotificationText.Type.None

    property alias title: titleText.text
    property alias notificationText: notificationText

    height: mainLayout.height + 24
    radius: 8
    visible: false

    onTypeChanged: {
        switch(type) {
            case NotificationText.Type.Info:
                icon.source = Images.infoCircle;
                icon.color = Styles.textInfo;
                notificationText.color = Styles.textInfo;
                titleText.color = Styles.textInfo;
                root.color = Styles.notificationInfo;
                break;
            case NotificationText.Type.AuthenticationError:
                icon.source = Images.lock;
                icon.color = Styles.textError;
                notificationText.color = Styles.textError;
                titleText.color = Styles.textError;
                root.color = Styles.notificationError
                break;
            default:
                break;
        }
    }

    RowLayout {
        id: mainLayout

        width: root.width

        anchors {
            top: root.top
            left: root.left
            right: root.right
            margins: 12
        }

        Custom.SvgImage {
            id: icon

            visible: icon.source !== ""
            Layout.alignment: Qt.AlignTop
            sourceSize: Qt.size(16, 16)
        }

        ColumnLayout {
            Layout.fillWidth: true

            Custom.Text {
                id: titleText

                visible: titleText.text.length !== 0
                Layout.fillWidth: true
                font.bold: true
            }

            Custom.RichText {
                id: notificationText

                visible: notificationText.text.length !== 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font {
                    pixelSize: 12
                    family: "Inter"
                    styleName: "Medium"
                }
            }
        }
    }

}



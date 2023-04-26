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
        Error
    }

    property int type: HintText.Type.None

    property alias iconSource: icon.source
    property alias iconColor: icon.color
    property alias title: titleText.text
    property alias titleColor: titleText.color
    property alias text: hintText.text
    property alias textColor: hintText.color

    height: (titleText.visible ? titleText.height : 0)
            + (hintText.visible? hintText.height : 0)
            + ((titleText.visible && hintText.visible) ? mainLayout.spacing : 0)

    color: "transparent"
    visible: false

    onTypeChanged: {
        switch(type) {
            case HintText.Type.Error:
                iconSource = Images.alertTriangle;
                iconColor = Styles.textError;
                titleColor = Styles.textError;
                textColor = Styles.textError;
                break;
            default:
                break;
        }
    }

    RowLayout {
        id: mainLayout

        width: root.width
        spacing: 8
        anchors {
            left: root.left
            top: root.top
        }

        Custom.SvgImage {
            id: icon

            visible: iconSource != ""
            Layout.alignment: Qt.AlignTop
            sourceSize: Qt.size(16, 16)
            Layout.preferredWidth: sourceSize.width
            Layout.preferredHeight: sourceSize.height
        }

        ColumnLayout {
            Layout.fillWidth: true

            Custom.RichText {
                id: titleText

                visible: title.length !== 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font {
                    bold: true
                    pixelSize: 12
                    weight: Font.Light
                    family: "Inter"
                    styleName: "Medium"
                }
            }

            Custom.RichText {
                id: hintText

                color: Styles.textSecondary
                visible: text.length !== 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font {
                    pixelSize: 12
                    weight: Font.Light
                    family: "Inter"
                    styleName: "Medium"
                }
            }
        }
    }

}



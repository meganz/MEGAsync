import QtQuick 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0 as Texts

Item {
    id: root

    property alias icon: hintIcon.source
    property alias title: hintTitle.rawText
    property alias text: hintText.rawText
    property alias iconColor: hintIcon.color
    property alias titleColor: hintTitle.color
    property alias textColor: hintText.color

    property int type: Constants.MessageType.NONE
    property int textSize: Texts.Text.Size.Normal

    implicitHeight: mainRow.height

    onTypeChanged: {
        switch(type) {
            case Constants.MessageType.NONE:
            case Constants.MessageType.SUCCESS:
            case Constants.MessageType.INFO:
                console.warn("HintText: Constants.MessageType -> " + type + " not defined yet");
                break;
            case Constants.MessageType.WARNING:
                if(icon.length === 0) {
                    icon = Images.alertTriangle;
                }
                iconColor = Styles.textWarning;
                titleColor = Styles.textWarning;
                textColor = Styles.textWarning;
                break;
            case Constants.MessageType.ERROR:
                if(icon.length === 0) {
                    icon = Images.xCircle;
                }
                iconColor = Styles.textError;
                titleColor = Styles.textError;
                textColor = Styles.textError;
                break;
            default:
                console.error("HintText: Constants.MessageType -> " + type + " does not exist");
                break;
        }
    }

    Row {
        id: mainRow

        height: root.visible ? textColumn.implicitHeight : 0
        spacing: root.icon !== "" ? 8 : 0
        width: root.width

        SvgImage {
            id: hintIcon

            sourceSize: Qt.size(16, 16)
            opacity: enabled ? 1.0 : 0.2
        }

        Column {
            id: textColumn

            anchors.top: parent.top
            width: mainRow.width - hintIcon.width - mainRow.spacing

            Texts.RichText {
                id: hintTitle

                height: rawText !== "" ? implicitHeight : 0
                width: parent.width
                opacity: enabled ? 1.0 : 0.2
                wrapMode: Text.WordWrap
                font{
                    bold: true
                    pixelSize: root.textSize
                }
            }

            Texts.RichText {
                id: hintText

                height: rawText !== "" ? implicitHeight : 0
                width: parent.width
                opacity: enabled ? 1.0 : 0.2
                font.pixelSize: root.textSize
                wrapMode: Text.WordWrap
                url: Links.contact
                manageMouse: true
            }
        }

    } // Row: mainRow

}

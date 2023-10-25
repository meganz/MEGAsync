// System
import QtQuick 2.15

// Local
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Common 1.0

Item {
    id: root

    property alias icon: hintIcon.source
    property alias title: hintTitle.rawText
    property alias text: hintText.rawText
    property alias iconColor: hintIcon.color
    property alias titleColor: hintTitle.color
    property alias textColor: hintText.color

    property int type: Constants.MessageType.NONE
    property int textSize: MegaTexts.Text.Size.Normal

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

    implicitHeight: row.height

    Row {
        id: row

        height: visible ? col.implicitHeight : 0
        spacing: root.icon !== "" ? 8 : 0
        width: root.width

        MegaImages.SvgImage {
            id: hintIcon

            sourceSize: Qt.size(16, 16)
            opacity: enabled ? 1.0 : 0.2
        }

        Column {
            id: col

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: row.width - hintIcon.width - row.spacing

            MegaTexts.RichText {
                id: hintTitle

                width: parent.width
                height: text !== "" ? implicitHeight : 0
                opacity: enabled ? 1.0 : 0.2
                font.bold: true
                font.pixelSize: root.textSize
                wrapMode: Text.WordWrap
            }

            MegaTexts.RichText {
                id: hintText

                width: parent.width
                height: text !== "" ? implicitHeight : 0
                opacity: enabled ? 1.0 : 0.2
                font.pixelSize: root.textSize
                wrapMode: Text.WordWrap
                url: Links.contact
                manageMouse: true
            }
        }
    }
}

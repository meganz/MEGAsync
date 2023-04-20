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
        InfoWithoutIcon,
        Help,
        Error,
        AuthenticationError
    }

    property int type: HintText.Type.None

    property alias iconSource: icon.source
    property alias iconColor: icon.color
    property alias title: titleText.text
    property alias titleColor: titleText.color
    property alias description: descriptionText.text
    property alias descriptionColor: descriptionText.color

    height: mainLayout.height
    color: "transparent"
    visible: type !== HintText.Type.None

    onTypeChanged: {
        switch(type) {
            case HintText.Type.Error:
                iconSource = Images.alertTriangle;
                iconColor = Styles.textError;
                titleColor = Styles.textError;
                descriptionColor = Styles.textError;
                break;
            case HintText.Type.Help:
                iconSource = Images.helpCircle;
                iconColor = Styles.textInfo;
                titleColor = Styles.textInfo;
                descriptionColor = Styles.textInfo;
                break;
            case HintText.Type.AuthenticationError:
                iconSource = Images.lock;
                iconColor = Styles.textError;
                titleColor = Styles.textError;
                descriptionColor = Styles.textError;
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

            visible: iconSource != "" && (title.length !== 0 || description.length !== 0)
            Layout.alignment: Qt.AlignTop
            sourceSize: Qt.size(16, 16)
        }

        ColumnLayout {
            Layout.fillWidth: true

            Text {
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

            Text {
                id: descriptionText

                visible: description.length !== 0
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



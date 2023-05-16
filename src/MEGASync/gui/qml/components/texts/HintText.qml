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
        Error,
        PasswordStrengthVeryWeak,
        PasswordStrengthWeak,
        PasswordStrengthMedium,
        PasswordStrengthGood,
        PasswordStrengthStrong
    }

    property int type: HintText.Type.None
    property url iconSource: ""
    property color iconColor
    property string title: ""
    property color titleColor
    property string text: ""
    property color textColor

    height: visible ? titleLoader.height + textLoader.height : 0
    color: "transparent"
    visible: false

    onTypeChanged: {
        var aa = type;
        switch(type) {
            case HintText.Type.Error:
                iconSource = Images.alertTriangle;
                iconColor = Styles.textError;
                titleColor = Styles.textError;
                textColor = Styles.textError;
                break;
            case HintText.Type.PasswordStrengthVeryWeak:
                iconSource = Images.passwordVeryWeak;
                iconColor = Styles.indicatorPink;
                titleColor = Styles.textError;
                textColor = Styles.textSecondary;
                break;
            case HintText.Type.PasswordStrengthWeak:
                iconSource = Images.passwordWeak;
                iconColor = Styles.supportError;
                titleColor = Styles.textError;
                textColor = Styles.textSecondary;
                break;
            case HintText.Type.PasswordStrengthMedium:
                iconSource = Images.passwordAverage;
                iconColor = Styles.supportWarning;
                titleColor = Styles.supportWarning;
                textColor = Styles.textSecondary;
                break;
            case HintText.Type.PasswordStrengthGood:
                iconSource = Images.passwordGood;
                iconColor = Styles.supportSuccess;
                titleColor = Styles.textSuccess;
                textColor = Styles.textSecondary;
                break;
            case HintText.Type.PasswordStrengthStrong:
                iconSource = Images.passwordStrong;
                iconColor = Styles.supportSuccess;
                titleColor = Styles.supportSuccess;
                textColor = Styles.textSecondary;
                break;
            default:
                break;
        }
    }

    onIconSourceChanged: {
        if(iconSource === "") {
            return;
        }

        iconLoader.sourceComponent = iconComponent;
    }

    onTitleChanged: {
        if(title.length === 0) {
            return;
        }

        titleLoader.sourceComponent = titleComponent;
    }

    onTextChanged: {
        if(text.length === 0) {
            return;
        }

        textLoader.sourceComponent = textComponent;
    }

    Loader {
        id: iconLoader
    }

    Column {
        anchors.left: iconLoader.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8

        spacing: 4

        Loader {
            id: titleLoader

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 0
            anchors.rightMargin: 0
        }

        Loader {
            id: textLoader

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.leftMargin: 0
        }
    }

    Component {
        id: iconComponent

        Custom.SvgImage {
            source: iconSource
            color: iconColor
            sourceSize: Qt.size(16, 16)
        }
    }

    Component {
        id: titleComponent

        Custom.Text {
            text: title
            color: titleColor
            opacity: enabled ? 1.0 : 0.2
            font.bold: true
            font.weight: Font.Light
        }
    }

    Component {
        id: textComponent

        Custom.Text {
            text: root.text
            color: textColor
            opacity: enabled ? 1.0 : 0.2
            font.weight: Font.Light
        }
    }
}



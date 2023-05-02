// System
import QtQuick 2.12

// Local
import Common 1.0

Text {
    id: control

    readonly property url defaultUrl: "default"

    property url url: defaultUrl
    property color urlColor: Styles.linkPrimary

    color: enabled ? Styles.textPrimary : Styles.textDisabled
    textFormat: Text.RichText
    font.pixelSize: 14

    Component.onCompleted: {
        control.text = control.text.replace("[b]","<b>")
        control.text = control.text.replace("[/b]","</b>")
        control.text = control.text.replace("[a]","<b>
                                            <a style=\"text-decoration:none\"
                                            style=\"color:" + urlColor + ";\" href=\"" + url + "\">")
        control.text = control.text.replace("[/a]","</a></b></font>")
    }

    onLinkActivated: {
        if(url != defaultUrl) {
            Qt.openUrlExternally(url);
        }
    }
}

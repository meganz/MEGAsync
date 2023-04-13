// System
import QtQuick 2.12

// Local
import Common 1.0

Text {
    property string url: url
    id: control
    color: Styles.textColor
    textFormat: Text.RichText
    font.pixelSize: 14
    Component.onCompleted: {
        control.text = control.text.replace("[b]","<b>")
        control.text = control.text.replace("[/b]","</b>")
        control.text = control.text.replace("[a]","<b>
                                            <a style=\"text-decoration:none\"
                                            style=\"color:" + Styles.linkPrimary + ";\" href=\"" + url + "\">")
        control.text = control.text.replace("[/a]","</a></b></font>")
    }
    onLinkActivated: Qt.openUrlExternally(url)
}


// System
import QtQuick 2.12 as Qml

// Local
import Common 1.0

Qml.Text {

    enum Size {
        Small = 10,
        Normal = 12,
        Medium = 14,
        MediumLarge = 16,
        Large = 20,
        Huge = 48
    }

    font.family: Constants.fontFamily
    font.styleName: Constants.fontStyleName
    font.pixelSize: Text.Size.Normal
    color: enabled ? Styles.textPrimary : Styles.textDisabled
    wrapMode: Text.WordWrap

}

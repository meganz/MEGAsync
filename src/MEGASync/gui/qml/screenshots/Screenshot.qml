import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    readonly property bool mirrored: LayoutMirroring.enabled
    readonly property int textAlignment: mirrored ? Text.AlignRight : Text.AlignLeft

    readonly property int dialogWidth: 480
    readonly property int dialogHeight: 396
    readonly property int contentMargin: 24
    readonly property int imageSize: 200
    readonly property int contentSpacing: 12
    readonly property int textSpacing: 8
    readonly property int buttonSpacing: 12
    readonly property int titleLineHeight: 28
    readonly property int descriptionLineHeight: 20

    title: ScreenshotStrings.title
    visible: false
    modality: Qt.NonModal
    closeOnEscapePressed: true
    color: ColorTheme.pageBackground
    width: dialogWidth
    height: dialogHeight
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    Column {
        id: contentColumn

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: window.contentMargin
            rightMargin: window.contentMargin
        }

        spacing: window.contentSpacing

        Image {
            id: illustrationImage

            anchors.horizontalCenter: parent.horizontalCenter
            width: window.imageSize
            height: window.imageSize
            source: Images.screenshotsUpload
        }

        Column {
            id: textColumn

            width: parent.width
            spacing: window.textSpacing

            Texts.Text {
                id: titleText

                width: parent.width
                text: ScreenshotStrings.offerTitle
                font.pixelSize: Texts.Text.Size.LARGE_SMALL
                font.weight: Font.DemiBold
                lineHeight: window.titleLineHeight
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: window.textAlignment
            }

            Texts.Text {
                id: descriptionText

                width: parent.width
                text: ScreenshotStrings.offerDescription
                font.pixelSize: Texts.Text.Size.MEDIUM
                lineHeight: window.descriptionLineHeight
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: window.textAlignment
            }
        }
    }

    Row {
        id: buttonsRow

        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: window.contentMargin + Constants.focusAdjustment
            bottomMargin: window.contentMargin + Constants.focusAdjustment
        }

        spacing: window.buttonSpacing + 2 * Constants.focusAdjustment

        OutlineButton {
            id: notNowButton

            text: ScreenshotStrings.notNow
            onClicked: {
                window.reject();
            }
        }

        PrimaryButton {
            id: enableButton

            text: ScreenshotStrings.enableScreenshots
            onClicked: {
                screenshotComponentAccess.enableScreenshotUploads();
                window.accept();
            }
        }
    }
}

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    readonly property int contentMargins: 64
    readonly property int gridButtonSpacing: 48
    readonly property int elementsSpacing: 16
    property int columnCount: repeater.count < 4 ? 1 : 2
    property int rowCount: repeater.count < 4 ? 1 : 2


    function adjustSize() {
        let windowWidth  = grid.width + contentMargins * 2
        let windowHeight  = grid.height + contentMargins * 2 + acceptButton.height + Constants.focusAdjustment + gridButtonSpacing
        window.width = windowWidth
        window.height = windowHeight
        window.minimumWidth = windowWidth
        window.minimumHeight = windowHeight
        window.maximumWidth = windowWidth
        window.maximumHeight = windowHeight
    }

    color: ColorTheme.surface1
    title: WhatsNewStrings.whatsNew

    Component.onCompleted: {
        x: Math.round((Screen.desktopAvailableWidth - width) / 2)
        y: Math.round((Screen.desktopAvailableHeight - height) / 2)
    }

    Rectangle {
        id: content

        anchors {
            fill: parent
            margins: contentMargins
        }
        color: 'transparent'
        Grid {
            id: grid

            rows: repeater.count < 4? 1 : 2
            columns: repeater.count < 4 ? repeater.count :2
            rowSpacing: 16
            columnSpacing: 16

            Repeater {
                id: repeater
                model: updatesModelAccess

                UpdatesElement {
                    id: element

                    imageSource: model? Images.imagesQmlPath + model.image + ".png" : ""
                    titleText: model? model.title : ""
                    descriptionText: model? model.description : ""
                }
            }
            onWidthChanged: {
                adjustSize();
            }
        }

        PrimaryButton {
            id: acceptButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Constants.focusAdjustment
            text: whatsNewWindowAccess.acceptButtonText()
            focus: true
            onClicked: {
                whatsNewWindowAccess.acceptButtonClicked();
                window.close();
            }
        }
    }
}

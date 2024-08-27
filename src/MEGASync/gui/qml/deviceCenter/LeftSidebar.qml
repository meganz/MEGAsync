import QtQuick 2.0

import common 1.0

import components.images 1.0
import components.texts 1.0 as Texts

import DeviceModel 1.0

Item {
    id:root

    readonly property int titleMargins: 24
    readonly property int itemHeight: 32

    Rectangle {
        id: contentItem

        anchors.fill: parent
    }
    Texts.RichText {
        id: deviceCenterTitle

        height: 30

        anchors {
            left: parent.left
            top: parent.top
            topMargin: titleMargins
            leftMargin: titleMargins
        }
        wrapMode: Text.NoWrap
        text: DeviceCenterStrings.deviceCenterWindowTitle
        font {
            pixelSize: Texts.Text.Size.LARGE
            weight: Font.Bold
        }
    }

    ListView {
        id: deviceList

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            top: deviceCenterTitle.bottom
            topMargin: 42
            leftMargin: titleMargins/2
            rightMargin: titleMargins/2
        }

        spacing: 8

        model: deviceCenterAccess.deviceModel
        delegate: deviceListDelegate
    }

    Component {
        id: deviceListDelegate

        Item {
            width: deviceList.width;
            height: itemHeight + 8
            Column {
                Rectangle {
                    id: deviceComponent

                    width: 232
                    height: itemHeight

                    color: colorStyle.surface1;
                    radius: 6;

                    Rectangle {
                        id: selectionIndicator

                        width: 4
                        height: 24
                        radius: 2
                        color: colorStyle.buttonBrand

                        anchors.left: deviceComponent.left
                        anchors.verticalCenter: deviceComponent.verticalCenter
                        anchors.leftMargin: 4
                    }
                    SvgImage {
                        id: deviceIcon

                        anchors.left: selectionIndicator.right
                        anchors.verticalCenter: deviceComponent.verticalCenter
                        anchors.leftMargin: 8

                        source: Images.devices
                        sourceSize: Qt.size(16, 16)
                    }
                    Texts.RichText {

                        anchors.left: deviceIcon.right
                        anchors.right: deviceComponent.right
                        anchors.verticalCenter: deviceComponent.verticalCenter
                        anchors.leftMargin: 8

                        font {
                            pixelSize: Texts.Text.Size.NORMAL
                            weight: Font.DemiBold
                        }
                        color: colorStyle.textPrimary

                        text: model ? model.name : ""
                    }
                }
            }
        }
    }
}

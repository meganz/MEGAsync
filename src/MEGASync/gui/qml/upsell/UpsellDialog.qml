import QtQuick 2.15
import QtQuick.Controls 2.15

import common 1.0

import components.images 1.0
import components.texts 1.0
import components.buttons 1.0

import QmlDialog 1.0

QmlDialog {
    id: window

    readonly property int contentMargin: 24

    title: UpsellStrings.dialogTitle
    visible: true
    modality: Qt.NonModal
    width: contentItem.width + 2 * window.contentMargin
    height: contentItem.height + 2 * window.contentMargin
    maximumWidth: contentItem.width + 2 * window.contentMargin
    maximumHeight: contentItem.height + 2 * window.contentMargin
    minimumWidth: contentItem.width + 2 * window.contentMargin
    minimumHeight: contentItem.height + 2 * window.contentMargin

    Column {
        id: contentItem

        anchors {
            left: parent.left
            top: parent.top
            topMargin: window.contentMargin
            leftMargin: window.contentMargin
            rightMargin: window.contentMargin
            bottomMargin: window.contentMargin - Constants.focusBorderWidth
        }
        width: 664
        height: 521
        spacing: 16

        Rectangle {
            id: header

            width: parent.width
            height: 168
            color: ColorTheme.surface2
            radius: 6

            Row {
                id: headerRow

                anchors {
                    fill: parent
                    verticalCenter: parent.verticalCenter
                    leftMargin: 24
                    rightMargin: 32
                }
                spacing: 40

                SvgImage {
                    id: imageItem

                    anchors.verticalCenter: parent.verticalCenter
                    sourceSize: Qt.size(104, 104)
                    source: Images.warning
                }

                Column {
                    id: titleColumn

                    anchors.verticalCenter: parent.verticalCenter
                    width: 464
                    spacing: 6

                    Text {
                        id: titleItem

                        width: parent.width
                        text: "Your MEGA cloud storage is full"
                        font {
                            pixelSize: Text.Size.LARGE
                            weight: Font.DemiBold
                        }
                    }

                    SecondaryText {
                        id: subtitleItem

                        width: parent.width
                        text: "Upgrade to Pro to get more storage quota. Or delete some files and empty your rubbish bin to free up storage space."
                        wrapMode: Text.Wrap
                        lineHeight: 18
                        lineHeightMode: Text.FixedHeight
                    }
                }
            }
        }

        Row {
            id: billedRow

            spacing: 12
            anchors.horizontalCenter: parent.horizontalCenter
            height: comboBoxRow.height

            Row {
                id: comboBoxRow

                spacing: 28

                RadioButton {
                    id: billedMonthlyRadioButton

                    text: "Billed monthly"
                    checked: true
                    ButtonGroup.group: buttonGroupItem
                }

                RadioButton {
                    id: billedYearlyRadioButton

                    text: "Billed yearly"
                    ButtonGroup.group: buttonGroupItem
                }
            }

            Rectangle {
                id: billedButton

                anchors.verticalCenter: parent.verticalCenter
                width: saveUpText.width + 8
                height: saveUpText.height + 4
                radius: 4
                color: ColorTheme.selectionControl

                Text {
                    id: saveUpText

                    anchors {
                        left: parent.left
                        top: parent.top
                        leftMargin: 4
                        rightMargin: 4
                        topMargin: 2
                        bottomMargin: 2
                    }
                    verticalAlignment: Text.AlignVCenter
                    font {
                        pixelSize: Text.Size.SMALL
                        weight: Font.DemiBold
                    }
                    lineHeight: 16
                    lineHeightMode: Text.FixedHeight
                    color: ColorTheme.textInverseAccent
                    text: "Save up to 16%"
                }
            }

        }

        Row {
            id: plansRow

            anchors {
                left: parent.left
                leftMargin: Constants.focusAdjustment
            }
            spacing: 0

            PlanCard {
                id: proLitePlan
            }

            PlanCard {
                id: proIPlan
            }

            PlanCard {
                id: proIIPlan
            }

            PlanCard {
                id: proIIIPlan
            }
        }

        Column {
            id: footerColumn

            width: parent.width
            spacing: 16

            SecondaryText {
                id: footerText

                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "* Estimated price in your local currency. Your account will be billed in Euros for all transactions."
            }

            Row {
                id: footerRow

                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 4

                SecondaryButton {
                    id: leftButton

                    text: "Not now"
                }

                PrimaryButton {
                    id: rightButton

                    text: "Buy Pro"
                }
            }
        }
    }

    ButtonGroup {
        id: buttonGroupItem
    }
}

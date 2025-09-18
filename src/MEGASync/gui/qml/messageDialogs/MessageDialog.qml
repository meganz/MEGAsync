import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.buttons 1.0
import components.checkBoxes 1.0
import components.images 1.0

import QmlDialog 1.0
import MessageDialogButtonInfo 1.0
import MessageDialogData 1.0

QmlDialog {
    id: window

    property QtObject messageDialogComponentAccess: instancesManager.instances["messageDialogComponentAccess"] || null
    property QtObject messageDialogDataAccess: instancesManager.instances["messageDialogDataAccess"] || null

    property MessageDialogMediumSizes sizes: MessageDialogMediumSizes {}

    property real totalWidth: Math.max(sizes.defaultMinimumWidth,
                                       contentColum.width + sizes.numberOfMargins * sizes.contentMargin)
    property real totalHeight: Math.max(sizes.defaultMinimumHeight,
                                        contentColum.height + sizes.numberOfMargins * sizes.contentMargin + Constants.focusAdjustment)
    width: window.totalWidth
    height: window.totalHeight
    flags: OS.isWindows() ? Qt.Dialog | Qt.MSWindowsFixedSizeDialogHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint : Qt.Dialog;
    //On Windows we don´t set a maximum width/height, so we set a non-reachable max size of the double of the required size (as a hack)
    //If we set a fixed size and the scale is not 100%, we found that the dialog is resized automatically by a few pixels.
    maximumWidth: OS.isWindows() ? window.totalWidth*2 : window.totalWidth
    maximumHeight: OS.isWindows() ? window.totalHeight*2 : window.totalHeight
    minimumWidth: OS.isWindows() ? 0 : window.totalWidth
    minimumHeight: OS.isWindows() ? 0 : window.totalHeight
    modality: Qt.WindowModal
    color: ColorTheme.surface1
    title: messageDialogDataAccess ? messageDialogDataAccess.title : ""
    visible: false

    onVisibleChanged:
    {
        if (window.visible) {
            setMessageDialogImageProperties();
        }
    }

    function setMessageDialogImageProperties()
    {
        if (messageDialogDataAccess !== null)
        {
            if (messageDialogDataAccess.type === MessageDialogData.Type.QUESTION)
            {
                imageItem.source = Images.dialogMessageQuestion;
                imageItem.color = ColorTheme.supportInfo;
            }
            else if (messageDialogDataAccess.type === MessageDialogData.Type.INFORMATION)
            {
                imageItem.source = Images.dialogMessageInformation;
                imageItem.color = ColorTheme.supportInfo;
            }
            else if (messageDialogDataAccess.type === MessageDialogData.Type.WARNING)
            {
                imageItem.source = Images.dialogMessageWarning;
                imageItem.color = ColorTheme.supportWarning;
            }
            else if (messageDialogDataAccess.type === MessageDialogData.Type.CRITICAL)
            {
                imageItem.source = Images.dialogMessageCritical;
                imageItem.color = ColorTheme.supportError;
            }
        }
    }

    Column {
        id: contentColum

        anchors {
            left: parent.left
            top: parent.top
            topMargin: sizes.contentMargin
            leftMargin: sizes.contentMargin
            rightMargin: sizes.contentMargin
            bottomMargin: sizes.contentMargin + Constants.focusAdjustment
        }
        width: topContentRow.width
        height: topContentRow.height + contentColum.spacing + bottomButtonsRow.height
        spacing: sizes.defaultSpacing

        Row {
            id: topContentRow

            width: imageItem.width + topContentRow.spacing + textColumn.width
            spacing: sizes.topContentRowSpacing

            SvgImage {
                id: imageItem

                width: sizes.iconSize
                height: sizes.iconSize
                sourceSize: Qt.size(sizes.iconSize, sizes.iconSize)
                visible: imageItem.source !== ""
            }

            Item {
                width: sizes.rightContentWidth
                height: Math.max(imageItem.height, textColumn.height)

                Column {
                    id: textColumn

                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    spacing: sizes.textColumnSpacing

                    TextLoader {
                        textInfo: messageDialogDataAccess ? messageDialogDataAccess.titleTextInfo : null
                        textLineHeight: sizes.titleTextLineHeight
                        textPixelSize: Texts.Text.Size.MEDIUM_LARGE
                        textWeight: Font.DemiBold
                    }

                    TextLoader {
                        textInfo: messageDialogDataAccess ? messageDialogDataAccess.descriptionTextInfo : null
                        textLineHeight: sizes.descriptionTextLineHeight
                        textPixelSize: Texts.Text.Size.NORMAL
                        textWeight: Font.Normal
                    }

                    CheckBox {
                        id: checkBoxItem
                        leftPadding: 0
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        text: messageDialogDataAccess ? messageDialogDataAccess.checkbox.text : ""
                        checked: messageDialogDataAccess ? messageDialogDataAccess.checkbox.checked : false
                        visible: checkBoxItem.text !== ""
                        onCheckedChanged: {
                            messageDialogComponentAccess.setChecked(checkBoxItem.checked);
                        }
                    }
                }

            }

        } // Row: topContentRow

        Item {
            anchors {
                left: parent.left
                right: parent.right
            }
            height: bottomButtonsRow.height

            Row {
                id: bottomButtonsRow

                anchors {
                    right: parent.right
                    rightMargin: Constants.focusAdjustment
                }
                spacing: sizes.bottomButtonsRowSpacing

                Repeater {
                    model: messageDialogDataAccess ? messageDialogDataAccess.buttons : []

                    Loader {
                        id: buttonLoader

                        property string buttonText: model.modelData ? model.modelData.text : ""
                        property url buttonIcon: model.modelData ? model.modelData.iconUrl : ""
                        property var onClicked: () => {
                            if (model.modelData) {
                                messageDialogComponentAccess.buttonClicked(model.modelData.type);
                            }
                            window.accept();
                        }

                        sourceComponent: {
                            switch(model.modelData.style) {
                                case MessageDialogButtonInfo.ButtonStyle.PRIMARY:
                                    return primaryButtonComponent;
                                case MessageDialogButtonInfo.ButtonStyle.SECONDARY:
                                    return secondaryButtonComponent;
                                case MessageDialogButtonInfo.ButtonStyle.LINK:
                                    return linkButtonComponent;
                                case MessageDialogButtonInfo.ButtonStyle.TEXT:
                                    return textButtonComponent;
                                case MessageDialogButtonInfo.ButtonStyle.OUTLINE:
                                default:
                                    return outlineButtonComponent;
                            }
                        }

                        Component {
                            id: primaryButtonComponent

                            PrimaryButton {
                                text: buttonLoader.buttonText
                                icons {
                                    source: buttonLoader.buttonIcon
                                    position: Icon.Position.LEFT
                                }
                                onClicked: buttonLoader.onClicked()
                            }
                        }

                        Component {
                            id: outlineButtonComponent

                            OutlineButton {
                                text: buttonLoader.buttonText
                                icons {
                                    source: buttonLoader.buttonIcon
                                    position: Icon.Position.LEFT
                                }
                                onClicked: buttonLoader.onClicked()
                            }
                        }

                        Component {
                            id: secondaryButtonComponent

                            SecondaryButton {
                                text: buttonLoader.buttonText
                                icons {
                                    source: buttonLoader.buttonIcon
                                    position: Icon.Position.LEFT
                                }
                                onClicked: buttonLoader.onClicked()
                            }
                        }

                        Component {
                            id: linkButtonComponent

                            LinkButton {
                                text: buttonLoader.buttonText
                                icons {
                                    source: buttonLoader.buttonIcon
                                    position: Icon.Position.LEFT
                                }
                                onClicked: buttonLoader.onClicked()
                            }
                        }

                        Component {
                            id: textButtonComponent

                            TextButton {
                                text: buttonLoader.buttonText
                                icons {
                                    source: buttonLoader.buttonIcon
                                    position: Icon.Position.LEFT
                                }
                                onClicked: buttonLoader.onClicked()
                            }
                        }

                    } // Loader: buttonLoader

                } // Repeater

            } // Row: bottomButtonsRow

        } // Item

    } // Column: contentColum

}

import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15

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

    // SNC-6567 (Phase 2a): The identifiers `messageDialogComponentAccess` and
    // `messageDialogDataAccess` are now provided as CHILD QQmlContext properties
    // registered by QmlDialogWrapper::create() BEFORE this QML tree is built
    // (see Phase 1 in QmlDialogWrapper.h). QML identifier resolution falls
    // through the local QML scope (which used to declare these as local
    // properties) to the child context, where they are guaranteed to be
    // non-null on the very first binding evaluation.
    //
    // Removing the previous local declarations:
    //
    //   property QtObject messageDialogComponentAccess:
    //       instancesManager.instances["messageDialogComponentAccess"] || null
    //   property QtObject messageDialogDataAccess:
    //       instancesManager.instances["messageDialogDataAccess"] || null
    //
    // eliminates the construction-time race where those expressions used to
    // read from an empty `instancesManager.instances` map and resolve to null,
    // leaving every dependent binding (icon, title, description, buttons,
    // checkbox) in its null/empty terminal state.

    property MessageDialogMediumSizes sizes: MessageDialogMediumSizes {}

    property real totalWidth: Math.min(sizes.defaultMaximumWidth, Math.max(sizes.defaultMinimumWidth, contentColum.implicitWidth + sizes.leftContentMargin + sizes.rightContentMargin))
    // SNC-6567: cap totalHeight with Math.min to break the implicit-height
    // feedback loop between contentColum.implicitHeight and the wrapping text's
    // implicitHeight that produced "Binding loop detected for property
    // totalHeight" QML warnings on every RichText dialog. Symmetrical with
    // the existing Math.min clamp on totalWidth.
    property real totalHeight: Math.min(sizes.defaultMaximumHeight, Math.max(sizes.defaultMinimumHeight, contentColum.implicitHeight + sizes.topContentMargin + sizes.bottomContentMargin))

    // Height the description is allowed to occupy before it starts scrolling.
    // It is the room left inside defaultMaximumHeight once the fixed pieces
    // (margins, title, footer, checkbox and the buttons row) are subtracted, so a
    // long description (e.g. the list of syncs in the "clear local cache" warning)
    // scrolls instead of overflowing the height-capped window and pushing the
    // buttons row out of view.
    //
    // These reference the siblings' implicitHeight (a layout INPUT derived from
    // their content), never their laid-out height (a layout OUTPUT): this value
    // feeds descriptionFlickable.Layout.preferredHeight, so reading the outputs
    // of the same layout pass would form the "Binding loop detected for property
    // totalHeight" cycle. implicitHeight is not rewritten by the vertical layout,
    // so the cycle is broken.
    property real maxDescriptionHeight: sizes.defaultMaximumHeight
                                        - sizes.topContentMargin - sizes.bottomContentMargin
                                        - (title.visible ? title.implicitHeight + sizes.textColumnSpacing : 0)
                                        - (footer.visible ? footer.implicitHeight + sizes.textColumnSpacing : 0)
                                        - sizes.defaultSpacing - bottomButtonsRow.implicitHeight
                                        - (checkBoxItem.visible ? checkBoxItem.implicitHeight + sizes.defaultSpacing : 0)

    width: window.totalWidth
    height: window.totalHeight
    flags: OS.isWindows() ? Qt.Dialog | Qt.MSWindowsFixedSizeDialogHint | Qt.WindowTitleHint | Qt.WindowCloseButtonHint : Qt.Dialog;
    //On Windows we don´t set a maximum width/height, so we set a non-reachable max size of the double of the required size (as a hack)
    //If we set a fixed size and the scale is not 100%, we found that the dialog is resized automatically by a few pixels.
    maximumWidth: OS.isWindows() ? window.totalWidth * 2 : window.totalWidth
    maximumHeight: OS.isWindows() ? window.totalHeight * 2 : window.totalHeight
    minimumWidth: OS.isWindows() ? 0 : window.totalWidth
    minimumHeight: OS.isWindows() ? 0 : window.totalHeight
    modality: Qt.WindowModal
    color: ColorTheme.surface1
    title: messageDialogDataAccess ? messageDialogDataAccess.title : ""
    visible: false
    closeOnEscapePressed: true

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
            if (messageDialogDataAccess.type === MessageDialogData.Type.SUCCESS)
            {
                imageItem.source = Images.dialogMessageSuccess;
                imageItem.color = ColorTheme.supportSuccess;
            }
            else if (messageDialogDataAccess.type === MessageDialogData.Type.QUESTION)
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

    ColumnLayout {
        id: contentColum

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top

            topMargin: sizes.topContentMargin
            leftMargin: sizes.leftContentMargin
            rightMargin: sizes.rightContentMargin
            bottomMargin: sizes.bottomContentMargin
        }

        spacing: sizes.defaultSpacing

        RowLayout {
            id: topContentRow

            Layout.fillWidth: true
            spacing: sizes.topContentRowSpacing

            SvgImage {
                id: imageItem

                width: sizes.iconSize
                height: sizes.iconSize
                Layout.preferredHeight: height
                Layout.preferredWidth: width
                Layout.alignment: Qt.AlignTop
                sourceSize: Qt.size(sizes.iconSize, sizes.iconSize)
                visible: imageItem.source !== ""
            }

            ColumnLayout {
                id: textColumn

                Layout.fillWidth: true
                spacing: sizes.textColumnSpacing

                TextLoader {
                    id: title

                    textInfo: messageDialogDataAccess ? messageDialogDataAccess.titleTextInfo : null
                    textLineHeight: sizes.titleTextLineHeight
                    textPixelSize: Texts.Text.Size.MEDIUM_LARGE
                    textWeight: Font.DemiBold
                }

                Flickable {
                    id: descriptionFlickable

                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(description.implicitHeight, window.maxDescriptionHeight)
                    Layout.alignment: title.visible ? Qt.AlignBottom : Qt.AlignVCenter

                    contentWidth: width
                    contentHeight: description.implicitHeight
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true

                    Qml.ScrollBar.vertical: Qml.ScrollBar {
                        id: descriptionScrollBar

                        policy: descriptionFlickable.contentHeight > descriptionFlickable.height
                                ? Qml.ScrollBar.AlwaysOn
                                : Qml.ScrollBar.AsNeeded
                    }

                    TextLoader {
                        id: description

                        // Leave a gutter on the right so the wrapped text does
                        // not run underneath the vertical scroll bar.
                        width: descriptionFlickable.width - descriptionScrollBar.width
                        textInfo: messageDialogDataAccess ? messageDialogDataAccess.descriptionTextInfo : null
                        textLineHeight: sizes.descriptionTextLineHeight
                        textPixelSize: Texts.Text.Size.NORMAL
                        textWeight: Font.Normal
                    }
                }

                TextLoader {
                    id: footer

                    textInfo: messageDialogDataAccess ? messageDialogDataAccess.footerTextInfo : null
                    textLineHeight: sizes.descriptionTextLineHeight
                    textPixelSize: Texts.Text.Size.NORMAL
                    textWeight: Font.Normal
                }
            }
        }

        Row {
            id: checkBoxRow

            Layout.fillWidth: true
            spacing: sizes.topContentRowSpacing
            visible: checkBoxItem.visible

            Item {
                id: spacer

                visible: checkBoxItem.visible
                width: sizes.iconSize
                height: sizes.iconSize
            }

            CheckBox {
                id: checkBoxItem

                leftPadding: 0
                text: messageDialogDataAccess ? messageDialogDataAccess.checkbox.text : ""
                checked: messageDialogDataAccess ? messageDialogDataAccess.checkbox.checked : false
                visible: checkBoxItem.text !== ""
                onCheckedChanged: {
                    messageDialogComponentAccess.setChecked(checkBoxItem.checked);
                }
            }
        }

        Row {
            id: bottomButtonsRow

            Layout.fillWidth: true
            spacing: sizes.bottomButtonsRowSpacing - 2 * Constants.focusBorderWidth
            layoutDirection: Qt.RightToLeft

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

    } // Column: contentColum
}

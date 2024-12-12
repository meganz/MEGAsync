import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

FocusScope {
    id: root

    property alias text: textArea.text
    property alias placeholderText: textArea.placeholderText
    property alias hint: hintItem

    property bool error: false
    property bool allowLineBreaks: true
    property int maxCharLength: -1

    property MediumSizes sizes: MediumSizes {}
    property Colors colors: Colors {}

    signal backPressed
    signal pastePressed
    signal returnPressed
    signal editingFinished

    function getMaxLenghtCharsString(value) {
        return qsTr("Maximum length is %n characters", "", value)
    }

    height: contentRect.height + hintItem.height + sizes.hintTopMargin
    Layout.preferredHeight: height

    Rectangle {
        id: contentRect

        anchors {
            left: parent.left
            right: parent.right
        }
        height: backgroundRect.height + 2 * sizes.focusBorderWidth
        radius: sizes.focusBorderRadius
        color: "transparent"
        border {
            color: textArea.activeFocus ? colors.focus : "transparent"
            width: sizes.focusBorderWidth
        }

        Rectangle {
            id: backgroundRect

            function getBorderColor() {
                var color = colors.border;
                if(!enabled) {
                    color = colors.borderDisabled;
                }
                else if(error) {
                    color = colors.borderError;
                }
                else if(textArea.activeFocus) {
                    color = colors.borderFocus;
                }
                return color;
            }

            anchors {
                top: contentRect.top
                left: contentRect.left
                right: contentRect.right
                rightMargin: sizes.focusBorderWidth
                topMargin: sizes.focusBorderWidth
                leftMargin: sizes.focusBorderWidth
            }

            height: flickableItem.height + 2 * sizes.verticalContentMargin
            radius: sizes.borderRadius
            color: colors.background
            border {
                color: getBorderColor()
                width: sizes.borderWidth
            }

            Flickable {
                id: flickableItem

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    rightMargin: sizes.rightContentMargin
                    topMargin: sizes.verticalContentMargin
                    bottomMargin: sizes.verticalContentMargin
                }
                height: sizes.adaptableHeight
                        ? Math.min(sizes.maxHeight, textArea.height)
                        : sizes.height
                contentWidth: flickableItem.width
                contentHeight: textArea.contentHeight + sizes.heightContentAdjustment
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                Qml.TextArea {
                    id: textArea

                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                        rightMargin: sizes.rightTextAreaMargin
                    }
                    wrapMode: Text.Wrap
                    selectByMouse: true
                    selectionColor: colors.selection
                    focus: true
                    placeholderTextColor: colors.placeholder
                    color: enabled ? colors.text : colors.textDisabled

                    font {
                        pixelSize: sizes.textSize
                        family: FontStyles.fontFamily
                        styleName: FontStyles.fontStyleName
                    }

                    onEditingFinished: root.editingFinished()

                    onTextChanged: {
                        if (root.maxCharLength > -1 && text.length > root.maxCharLength) {
                            var cursorPos = cursorPosition;
                            text = text.substring(0, root.maxCharLength);
                            cursorPosition = Math.min(cursorPos, root.maxCharLength);
                            hintItem.text = getMaxLenghtCharsString(root.maxCharLength);
                            hintItem.icon = Images.alertTriangle;
                            error = true;
                            hintItem.visible = true;
                        }
                        else {
                            error = false;
                            hintItem.visible = false;
                        }

                        if (flickableItem.contentHeight > flickableItem.height) {
                            flickableItem.contentY = flickableItem.contentHeight - flickableItem.height;
                        }
                    }

                    Keys.onPressed: {
                        if (event.key === Qt.Key_Backspace) {
                            root.backPressed();
                        }
                        else if ((event.key === Qt.Key_V) && (event.modifiers & Qt.ControlModifier)) {
                            pastePressed();
                        }
                        else if (event.key === Qt.Key_Return) {
                            if (!root.allowLineBreaks) {
                                event.accepted = true;
                                root.returnPressed();
                            }
                        }
                    }
                }

                Qml.ScrollBar.vertical: Qml.ScrollBar {
                    policy: Qml.ScrollBar.AsNeeded
                }

            }
        }
    }

    Texts.HintText {
        id: hintItem

        anchors {
            left: parent.left
            right: parent.right
            top: contentRect.bottom
            topMargin: sizes.hintTopMargin
            leftMargin: sizes.focusBorderWidth
            rightMargin: sizes.focusBorderWidth
        }
        type: Constants.MessageType.ERROR
        visible: false
    }

}

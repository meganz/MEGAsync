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
    property bool autoScrollOnArrows: true
    property int maxCharLength: -1

    property MediumSizes sizes: MediumSizes {}
    property Colors colors: Colors {}

    signal backPressed
    signal pastePressed
    signal returnPressed
    signal editingFinished

    function getMaxLenghtCharsString(value) {
        return qsTr("Maximum length is %n characters.", "", value)
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

                    function ensureCursorVisible() {
                        // Calculation based on the number of line breaks.
                        var cursorPos = textArea.cursorPosition;
                        var textBeforeCursor = textArea.text.substring(0, cursorPos);

                        // Current line (0-indexed).
                        var currentLine = textBeforeCursor.split("\n").length;

                        // Total height to current line.
                        var lineHeight = sizes.textSize + sizes.lineHeight;
                        var cursorHeight = (currentLine + 1) * lineHeight;

                        if (cursorHeight > flickableItem.contentY + flickableItem.height) {
                            flickableItem.contentY = cursorHeight - flickableItem.height + lineHeight;
                        }
                        else if (cursorHeight < flickableItem.contentY) {
                            flickableItem.contentY = Math.max(cursorHeight - lineHeight, 0);
                        }
                    }

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
                            }
                            root.returnPressed();
                            textArea.ensureCursorVisible();
                        }
                        else if (event.key === Qt.Key_Up) {
                            if (root.autoScrollOnArrows) {
                                var currentCursorPos = textArea.cursorPosition;
                                var textBeforeCursor = textArea.text.substring(0, currentCursorPos);

                                // Find the start and end of the current line.
                                var currentLineStart = textBeforeCursor.lastIndexOf('\n') + 1;
                                var previousLineStart = textBeforeCursor.lastIndexOf('\n', currentLineStart - 2) + 1;

                                // If there's a previous line.
                                if (previousLineStart >= 0) {
                                    var relativePos = currentCursorPos - currentLineStart;

                                    // Find the end of the previous line.
                                    var previousLineEnd = currentLineStart - 1;
                                    if (previousLineEnd < 0) {
                                        previousLineEnd = 0;
                                    }

                                    var previousLineLength = previousLineEnd - previousLineStart;

                                    // Adjust the cursor to the same relative position or the end of the previous line.
                                    var targetPos = previousLineStart + Math.min(relativePos, previousLineLength);
                                    textArea.cursorPosition = targetPos;

                                    // Calculate the line height and adjust the scroll.
                                    // Approximate height of a line.
                                    var lineHeight = sizes.textSize + sizes.lineHeight;
                                    var targetLine = textBeforeCursor.substring(0, targetPos).split('\n').length - 1;

                                    // Ensure contentY is within allowed bounds.
                                    var targetY = targetLine * lineHeight - flickableItem.height / 2;
                                    flickableItem.contentY = Math.min(Math.max(targetY, 0),
                                                                      flickableItem.contentHeight - flickableItem.height);
                                }
                            }
                            event.accepted = true;
                        }
                        else if (event.key === Qt.Key_Down) {
                            if (root.autoScrollOnArrows) {
                                var currentCursorPosDown = textArea.cursorPosition;
                                var textBeforeCursorDown = textArea.text.substring(0, currentCursorPosDown);

                                // Find the end of the current line.
                                var currentLineEnd = textArea.text.indexOf('\n', currentCursorPosDown);
                                if (currentLineEnd === -1) {
                                    currentLineEnd = textArea.text.length;
                                }

                                var nextLineStart = currentLineEnd + 1;
                                if (nextLineStart < textArea.text.length) {
                                    var relativePosDown = currentCursorPosDown - (textBeforeCursorDown.lastIndexOf('\n') + 1);

                                    // Find the end of the next line.
                                    var nextLineEnd = textArea.text.indexOf('\n', nextLineStart);
                                    if (nextLineEnd === -1) {
                                        nextLineEnd = textArea.text.length;
                                    }

                                    var nextLineLength = nextLineEnd - nextLineStart;

                                    // Adjust the cursor to the same relative position or the end of the next line
                                    var targetPosDown = nextLineStart + Math.min(relativePosDown, nextLineLength);
                                    textArea.cursorPosition = targetPosDown;

                                    // Calculate the line height and adjust the scroll.
                                    var lineHeightDown = sizes.textSize + sizes.lineHeight; // Approximate height of a line
                                    var targetLineDown = textBeforeCursorDown.substring(0, targetPosDown).split('\n').length - 1;

                                    // Ensure contentY is within allowed bounds.
                                    var targetYDown = targetLineDown * lineHeightDown - flickableItem.height / 2;
                                    flickableItem.contentY = Math.min(Math.max(targetYDown, 0),
                                                                      flickableItem.contentHeight - flickableItem.height);
                                }
                            }
                            event.accepted = true;
                        }
                        else if (event.key === Qt.Key_Right) {
                            if (root.autoScrollOnArrows) {
                                // Move the cursor to the right if we're not at the end of the text.
                                if (textArea.cursorPosition < textArea.text.length) {
                                    textArea.cursorPosition += 1;
                                }

                                // Adjust scroll based on cursor position.
                                var currentCursorPosRight = textArea.cursorPosition;
                                var textBeforeCursorRight = textArea.text.substring(0, currentCursorPosRight);

                                // Calculate the current line based on line breaks.
                                var currentLineRight = textBeforeCursorRight.split("\n").length - 1;

                                // Calculate line height and adjustment.
                                var lineHeightRight = sizes.textSize + sizes.lineHeight;
                                var cursorYRight = currentLineRight * lineHeightRight;

                                // Ensure the scroll does not exceed available content.
                                var maxContentY = flickableItem.contentHeight - flickableItem.height;
                                if (cursorYRight + lineHeightRight > flickableItem.contentY + flickableItem.height) {
                                    flickableItem.contentY = Math.min(cursorYRight + lineHeightRight - flickableItem.height, maxContentY);
                                } else if (cursorYRight < flickableItem.contentY) {
                                    flickableItem.contentY = Math.max(0, cursorYRight);
                                }
                            }
                            event.accepted = true;
                        }
                        else if (event.key === Qt.Key_Left) {
                            if (root.autoScrollOnArrows) {
                                // Move the cursor to the left if we're not at the beginning.
                                if (textArea.cursorPosition > 0) {
                                    textArea.cursorPosition -= 1;
                                }

                                // Adjust scroll based on cursor position.
                                var currentCursorPosLeft = textArea.cursorPosition;
                                var textBeforeCursorLeft = textArea.text.substring(0, currentCursorPosLeft);

                                // Calculate the current line based on line breaks.
                                var currentLineLeft = textBeforeCursorLeft.split("\n").length - 1;

                                // Calculate line height and adjustment.
                                var lineHeightLeft = sizes.textSize + sizes.lineHeight;
                                var cursorYLeft = currentLineLeft * lineHeightLeft;

                                // Ensure the scroll does not exceed the content limits.
                                var maxContentYLeft = flickableItem.contentHeight - flickableItem.height;
                                if (cursorYLeft + lineHeightLeft > flickableItem.contentY + flickableItem.height) {
                                    flickableItem.contentY = Math.min(cursorYLeft + lineHeightLeft - flickableItem.height, maxContentYLeft);
                                } else if (cursorYLeft < flickableItem.contentY) {
                                    flickableItem.contentY = Math.max(0, cursorYLeft);
                                }
                            }
                            event.accepted = true;
                        }
                    }
                }

                Qml.ScrollBar.vertical: Qml.ScrollBar {
                    id: scrollbar

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

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
                                var currentText = textArea.text;

                                // Find the start of the current line
                                var currentLineStart = currentText.lastIndexOf('\n', currentCursorPos - 1) + 1;
                                var previousLineStart = currentText.lastIndexOf('\n', currentLineStart - 2) + 1;

                                // If there is a previous line
                                if (previousLineStart >= 0) {
                                    var currentLineEnd = currentText.indexOf('\n', currentLineStart);
                                    if (currentLineEnd === -1) {
                                        currentLineEnd = currentText.length; // Last line of text
                                    }

                                    // Calculate the relative position within the current line
                                    var relativePos = currentCursorPos - currentLineStart;

                                    // Adjust the cursor to move exactly to the previous line
                                    var targetPos = Math.min(previousLineStart + relativePos, currentLineEnd - 1);

                                    // Get the size of the previous line
                                    var previousLineText = currentText.substring(previousLineStart, currentLineStart - 1);
                                    var previousLineSize = previousLineText.length;

                                    // If the previous line is smaller and the cursor position is outside the line,
                                    // move the cursor to the last character of the previous line
                                    if (previousLineSize < relativePos) {
                                        targetPos = previousLineStart + previousLineSize;
                                    }

                                    textArea.cursorPosition = targetPos;

                                    // Ensure the upper line is visible
                                    var cursorRect = textArea.cursorRectangle;
                                    var cursorYInFlickable = textArea.mapToItem(flickableItem, cursorRect.x, cursorRect.y).y;

                                    var lineHeight = sizes.textSize + sizes.lineHeight; // Adjust line height

                                    // Move only when the cursor is out of view at the top
                                    if (cursorYInFlickable < flickableItem.contentY) {
                                        flickableItem.contentY = Math.max(flickableItem.contentY - lineHeight, 0);
                                    }
                                }
                            }
                            event.accepted = true;
                        }
                        else if (event.key === Qt.Key_Down) {
                            if (root.autoScrollOnArrows) {
                                var currentCursorPosDown = textArea.cursorPosition;
                                var currentTextDown = textArea.text;

                                // Find the end of the current line
                                var currentLineEndDown = currentTextDown.indexOf('\n', currentCursorPosDown);
                                if (currentLineEndDown === -1) {
                                    currentLineEndDown = currentTextDown.length; // Last line of text
                                }

                                // Find the start of the next line
                                var nextLineStartDown = currentLineEndDown + 1;
                                if (nextLineStartDown >= currentTextDown.length) {
                                    nextLineStartDown = currentTextDown.length; // No more lines
                                }

                                // If there is a next line
                                if (nextLineStartDown < currentTextDown.length) {
                                    var currentLineStartDown = currentTextDown.lastIndexOf('\n', currentCursorPosDown - 1) + 1;
                                    // Calculate the relative position within the current line
                                    var relativePosDown = currentCursorPosDown - currentLineStartDown;

                                    // Get the text of the next line (excluding the newline character)
                                    var nextLineTextDown = currentTextDown.substring(nextLineStartDown); // Gets the text of the next line
                                    var nextLineSizeDown = nextLineTextDown.indexOf('\n') === -1 ? nextLineTextDown.length : nextLineTextDown.indexOf('\n'); // Avoid counting the newline

                                    // If the next line is smaller and the cursor position is outside the line,
                                    // move the cursor to the last character of the next line
                                    var targetPosDown = 0;
                                    if (nextLineSizeDown < relativePosDown) {
                                        // If the size of the next line is smaller than the relative position, move to the end of the next line
                                        targetPosDown = nextLineStartDown + nextLineSizeDown;
                                    }
                                    else {
                                        // Otherwise, adjust the cursor to the relative position within the new line
                                        targetPosDown = nextLineStartDown + relativePosDown;
                                    }

                                    // Update the cursor position
                                    textArea.cursorPosition = targetPosDown;

                                    // Ensure the next line is visible
                                    var cursorRectDown = textArea.cursorRectangle;
                                    var cursorYInFlickableDown = textArea.mapToItem(flickableItem, cursorRectDown.x, cursorRectDown.y).y;

                                    var lineHeightDown = sizes.textSize + sizes.lineHeight; // Adjust line height

                                    // Move only when the cursor is out of view at the bottom
                                    if (cursorYInFlickableDown > flickableItem.contentY) {
                                        flickableItem.contentY = Math.min(flickableItem.contentY + lineHeightDown,
                                                                          flickableItem.contentHeight - flickableItem.height);
                                    }
                                }
                            }
                            event.accepted = true;
                        }
                        else if (event.key === Qt.Key_Right) {
                            if (root.autoScrollOnArrows) {
                                var currentCursorPosRight = textArea.cursorPosition;
                                var currentTextRight = textArea.text;

                                // Find the end of the current line
                                var currentLineEndRight = currentTextRight.indexOf('\n', currentCursorPosRight);
                                if (currentLineEndRight === -1) {
                                    currentLineEndRight = currentTextRight.length; // End of text if no newline
                                }

                                // Check if the cursor is at the end of the current line
                                if (currentCursorPosRight === currentLineEndRight) {
                                    // Move to the next line
                                    var nextLineStartRight = currentLineEndRight + 1;
                                    if (nextLineStartRight >= currentTextRight.length) {
                                        nextLineStartRight = currentTextRight.length; // No more lines
                                    }

                                    // If there is a next line, move the cursor to the start of the next line
                                    if (nextLineStartRight < currentTextRight.length) {
                                        var nextLineTextRight = currentTextRight.substring(nextLineStartRight);
                                        var nextLineSizeRight = nextLineTextRight.indexOf('\n') === -1 ? nextLineTextRight.length : nextLineTextRight.indexOf('\n');

                                        // Set the target position to the start of the next line
                                        var targetPosRight = nextLineStartRight;

                                        textArea.cursorPosition = targetPosRight;

                                        // Ensure the next line is visible (scroll down if needed)
                                        var cursorRectRight = textArea.cursorRectangle;
                                        var cursorYInFlickableRight = textArea.mapToItem(flickableItem, cursorRectRight.x, cursorRectRight.y).y;

                                        var lineHeightRight = sizes.textSize + sizes.lineHeight;

                                        // Scroll only if the cursor is below the visible area
                                        if (cursorYInFlickableRight > flickableItem.contentY) {
                                            flickableItem.contentY = Math.min(flickableItem.contentY + lineHeightRight,
                                                                              flickableItem.contentHeight - flickableItem.height);
                                        }
                                    }
                                } else {
                                    // Default right arrow behavior: just move the cursor one character to the right
                                    textArea.cursorPosition = currentCursorPosRight + 1;
                                }
                            }
                            event.accepted = true;
                        }
                        else if (event.key === Qt.Key_Left) {
                            if (root.autoScrollOnArrows) {
                                var currentCursorPosLeft = textArea.cursorPosition;
                                var currentTextLeft = textArea.text;

                                // Find the start of the current line
                                var currentLineStartLeft = currentTextLeft.lastIndexOf('\n', currentCursorPosLeft - 1) + 1;

                                // Check if the cursor is at the start of the current line
                                if (currentCursorPosLeft === currentLineStartLeft) {
                                    // Move to the previous line
                                    var previousLineEndLeft = currentTextLeft.lastIndexOf('\n', currentLineStartLeft - 2) + 1;
                                    if (previousLineEndLeft === -1) {
                                        previousLineEndLeft = 0; // No more previous lines
                                    }

                                    // If there is a previous line, move the cursor to the last character of the previous line
                                    if (previousLineEndLeft >= 0) {
                                        var previousLineTextLeft = currentTextLeft.substring(previousLineEndLeft, currentLineStartLeft - 1);
                                        var previousLineSizeLeft = previousLineTextLeft.length;

                                        // Set the target position to the last character of the previous line
                                        var targetPosLeft = previousLineEndLeft + previousLineSizeLeft;

                                        textArea.cursorPosition = targetPosLeft;

                                        // Ensure the previous line is visible (scroll up if needed)
                                        var cursorRectLeft = textArea.cursorRectangle;
                                        var cursorYInFlickableLeft = textArea.mapToItem(flickableItem, cursorRectLeft.x, cursorRectLeft.y).y;

                                        var lineHeightLeft = sizes.textSize + sizes.lineHeight;

                                        // Scroll only if the cursor is above the visible area
                                        if (cursorYInFlickableLeft < flickableItem.contentY) {
                                            flickableItem.contentY = Math.max(flickableItem.contentY - lineHeightLeft, 0);
                                        }
                                    }
                                } else {
                                    // Default left arrow behavior: just move the cursor one character to the left
                                    textArea.cursorPosition = currentCursorPosLeft - 1;
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

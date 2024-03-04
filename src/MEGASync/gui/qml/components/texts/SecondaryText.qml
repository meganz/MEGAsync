import common 1.0

import components.texts 1.0 as Texts

Texts.Text {
    id: root

    color: enabled ? colorStyle.textSecondary : colorStyle.textDisabled
}

import common 1.0

import components.texts 1.0 as Texts

Texts.RichText {
    id: root

    color: enabled ? ColorTheme.textSecondary : ColorTheme.textDisabled
}

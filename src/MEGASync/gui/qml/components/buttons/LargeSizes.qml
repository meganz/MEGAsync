import components.texts 1.0 as Texts

Sizes {
    id: root

    horizontalPadding: borderLess ? 12 : 24
    spacing: 12
    radius: 8
    iconWidth: 22
    textFontSize: Texts.Text.Size.MediumLarge
    textLineHeight: 24
    focusBorderRadius: 12.5
}

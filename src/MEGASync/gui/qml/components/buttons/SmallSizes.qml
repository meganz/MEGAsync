import components.texts 1.0 as Texts

Sizes {
    id: root

    horizontalPadding: borderLess ? 4 : 8
    textFontSize: Texts.Text.Size.Normal
    radius: 6
    textLineHeight: 18
    spacing: horizontalPadding
}

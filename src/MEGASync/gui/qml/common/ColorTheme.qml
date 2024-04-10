pragma Singleton

import QtQuick 2.15

Item {
    id: root

    Loader{
        id: loader
        source: "qrc:/common/themes/"+themeManager.theme+"/Colors.qml"
    }

	readonly property color backgroundBlur: loader.item.backgroundBlur
	readonly property color backgroundInverse: loader.item.backgroundInverse
	readonly property color borderDisabled: loader.item.borderDisabled
	readonly property color borderInteractive: loader.item.borderInteractive
	readonly property color borderStrong: loader.item.borderStrong
	readonly property color borderStrongSelected: loader.item.borderStrongSelected
	readonly property color borderSubtle: loader.item.borderSubtle
	readonly property color borderSubtleSelected: loader.item.borderSubtleSelected
	readonly property color buttonBrand: loader.item.buttonBrand
	readonly property color buttonBrandHover: loader.item.buttonBrandHover
	readonly property color buttonBrandPressed: loader.item.buttonBrandPressed
	readonly property color buttonDisabled: loader.item.buttonDisabled
	readonly property color buttonError: loader.item.buttonError
	readonly property color buttonErrorHover: loader.item.buttonErrorHover
	readonly property color buttonErrorPressed: loader.item.buttonErrorPressed
	readonly property color buttonOutline: loader.item.buttonOutline
	readonly property color buttonOutlineBackgroundHover: loader.item.buttonOutlineBackgroundHover
	readonly property color buttonOutlineHover: loader.item.buttonOutlineHover
	readonly property color buttonOutlinePressed: loader.item.buttonOutlinePressed
	readonly property color buttonPrimary: loader.item.buttonPrimary
	readonly property color buttonPrimaryHover: loader.item.buttonPrimaryHover
	readonly property color buttonPrimaryPressed: loader.item.buttonPrimaryPressed
	readonly property color buttonSecondary: loader.item.buttonSecondary
	readonly property color buttonSecondaryHover: loader.item.buttonSecondaryHover
	readonly property color buttonSecondaryPressed: loader.item.buttonSecondaryPressed
	readonly property color divider: loader.item.divider
	readonly property color extInverse: loader.item.extInverse
	readonly property color focusColor: loader.item.focusColor
	readonly property color iconAccent: loader.item.iconAccent
	readonly property color iconButton: loader.item.iconButton
	readonly property color iconButtonDisabled: loader.item.iconButtonDisabled
	readonly property color iconButtonHover: loader.item.iconButtonHover
	readonly property color iconButtonPressed: loader.item.iconButtonPressed
	readonly property color iconButtonPressedBackground: loader.item.iconButtonPressedBackground
	readonly property color iconDisabled: loader.item.iconDisabled
	readonly property color iconInverse: loader.item.iconInverse
	readonly property color iconInverseAccent: loader.item.iconInverseAccent
	readonly property color iconInverseAccentHover: loader.item.iconInverseAccentHover
	readonly property color iconInverseAccentPlaceholder: loader.item.iconInverseAccentPlaceholder
	readonly property color iconInverseAccentPress: loader.item.iconInverseAccentPress
	readonly property color iconOnColor: loader.item.iconOnColor
	readonly property color iconOnColorDisabled: loader.item.iconOnColorDisabled
	readonly property color iconPlaceholder: loader.item.iconPlaceholder
	readonly property color iconPrimary: loader.item.iconPrimary
	readonly property color iconSecondary: loader.item.iconSecondary
	readonly property color indicatorBackground: loader.item.indicatorBackground
	readonly property color indicatorBlue: loader.item.indicatorBlue
	readonly property color indicatorGreen: loader.item.indicatorGreen
	readonly property color indicatorIndigo: loader.item.indicatorIndigo
	readonly property color indicatorMagenta: loader.item.indicatorMagenta
	readonly property color indicatorOrange: loader.item.indicatorOrange
	readonly property color indicatorPink: loader.item.indicatorPink
	readonly property color indicatorYellow: loader.item.indicatorYellow
	readonly property color interactive: loader.item.interactive
	readonly property color linkInverse: loader.item.linkInverse
	readonly property color linkPrimary: loader.item.linkPrimary
	readonly property color linkVisited: loader.item.linkVisited
	readonly property color notificationError: loader.item.notificationError
	readonly property color notificationInfo: loader.item.notificationInfo
	readonly property color notificationSuccess: loader.item.notificationSuccess
	readonly property color notificationWarning: loader.item.notificationWarning
	readonly property color pageBackground: loader.item.pageBackground
	readonly property color selectionControl: loader.item.selectionControl
	readonly property color supportError: loader.item.supportError
	readonly property color supportInfo: loader.item.supportInfo
	readonly property color supportSuccess: loader.item.supportSuccess
	readonly property color supportWarning: loader.item.supportWarning
	readonly property color surface1: loader.item.surface1
	readonly property color surface2: loader.item.surface2
	readonly property color surface3: loader.item.surface3
	readonly property color textAccent: loader.item.textAccent
	readonly property color textDisabled: loader.item.textDisabled
	readonly property color textError: loader.item.textError
	readonly property color textInfo: loader.item.textInfo
	readonly property color textInverse: loader.item.textInverse
	readonly property color textInverseAccent: loader.item.textInverseAccent
	readonly property color textInverseAccentHover: loader.item.textInverseAccentHover
	readonly property color textInverseAccentPlaceholder: loader.item.textInverseAccentPlaceholder
	readonly property color textInverseAccentPress: loader.item.textInverseAccentPress
	readonly property color textOnColor: loader.item.textOnColor
	readonly property color textOnColorDisabled: loader.item.textOnColorDisabled
	readonly property color textPlaceholder: loader.item.textPlaceholder
	readonly property color textPrimary: loader.item.textPrimary
	readonly property color textSecondary: loader.item.textSecondary
	readonly property color textSuccess: loader.item.textSuccess
	readonly property color textWarning: loader.item.textWarning
	readonly property color toastBackground: loader.item.toastBackground
}

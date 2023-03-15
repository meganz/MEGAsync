pragma Singleton
import QtQuick 2.12

QtObject {
    property bool lightTheme: true

    readonly property int width: 640
    readonly property int height: 480

    /* Edit this comment to add your custom font */
    readonly property font font: Qt.font({
                                             family: Qt.application.font.family,
                                             pixelSize: Qt.application.font.pixelSize
                                         })
    readonly property font largeFont: Qt.font({
                                                  family: Qt.application.font.family,
                                                  pixelSize: Qt.application.font.pixelSize * 1.6
                                              })

    readonly property color backgroundColor: lightTheme ? "#F6F6F6" : "#282828"
    readonly property color alternateBackgroundColor: lightTheme ? "#FFFFFF" : "#18191A"
    readonly property color textColor: lightTheme ? "#04101E" : "#FAFAFB"

    readonly property color pageBackground: lightTheme ? "#FFFFFF" : "#18191A"
    readonly property color surface1: lightTheme ? "#FAFAFA" : "#303233"
    readonly property color surface2: lightTheme ? "#F3F4F4" : "#494A4D"
    readonly property color surface3: lightTheme ? "#D8D9DB" : "#616366"
    readonly property color backgroundInverse: lightTheme ? "#494A4D" : "#F3F4F4"
    readonly property color borderInteractive: lightTheme ? "#DD1405" : "#F23433"
    readonly property color borderStrong: lightTheme ? "#D8D9DB" : "#616366"
    readonly property color borderStrongSelected: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color borderSubtle: lightTheme ? "#F3F4F4" : "#303233"
    readonly property color borderSubtleSelected: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color borderDisabled: lightTheme ? "#D8D9DB" : "#494A4D"
    readonly property color textPrimary: lightTheme ? "#303233" : "#F3F4F4"
    readonly property color textSecondary: lightTheme ? "#616366" : "#A9ABAD"
    readonly property color textAccent: lightTheme ? "#04101E" : "#FAFAFB"
    readonly property color textPlaceholder: lightTheme ? "#616366" : "#C1C2C4"
    readonly property color textInverseAccent: lightTheme ? "#FAFAFB" : "#04101E"
    readonly property color textOnColor: lightTheme ? "#FAFAFA" : "#FAFAFA"
    readonly property color textOnColorDisabled: lightTheme ? "#A9ABAD" : "#919397"
    readonly property color textError: lightTheme ? "#E31B57" : "#FD6F90"
    readonly property color textSuccess: lightTheme ? "#007C3E" : "#09BF5B"
    readonly property color textInfo: lightTheme ? "#0078A4" : "#05BAF1"
    readonly property color textWarning: lightTheme ? "#B55407" : "#F7A308"
    readonly property color textInverse: lightTheme ? "#FAFAFB" : "#303233"
    readonly property color textDisabled: lightTheme ? "#C1C2C4" : "#797C80"
    readonly property color linkPrimary: lightTheme ? "#2C5BEB" : "#69A3FB"
    readonly property color linkInverse: lightTheme ? "#69A3FB" : "#2C5BEB"
    readonly property color linkVisited: lightTheme ? "#233783" : "#D9E8FF"
    readonly property color iconPrimary: lightTheme ? "#303233" : "#F3F4F4"
    readonly property color iconSecondary: lightTheme ? "#616366" : "#A9ABAD"
    readonly property color iconAccent: lightTheme ? "#04101E" : "#FAFAFB"
    readonly property color iconInverseAccent: lightTheme ? "#FAFAFB" : "#04101E"
    readonly property color iconOnColor: lightTheme ? "#FAFAFA" : "#FAFAFA"
    readonly property color iconOnColorDisabled: lightTheme ? "#A9ABAD" : "#919397"
    readonly property color iconInverse: lightTheme ? "#FAFAFB" : "#303233"
    readonly property color iconDisabled: lightTheme ? "#C1C2C4" : "#797C80"
    readonly property color buttonPrimary: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color buttonPrimaryHover: lightTheme ? "#39424E" : "#A3A6AD"
    readonly property color buttonPrimaryPressed: lightTheme ? "#535B65" : "#BDC0C4"
    readonly property color buttonBrand: lightTheme ? "#DD1405" : "#F23433"
    readonly property color buttonBrandHover: lightTheme ? "#B61714" : "#FB6361"
    readonly property color buttonBrandPressed: lightTheme ? "#931715" : "#FD9997"
    readonly property color buttonSecondary: lightTheme ? "#F3F4F4" : "#494A4D"
    readonly property color buttonSecondaryPressed: lightTheme ? "#C1C2C4" : "#797C80"
    readonly property color buttonOutline: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color buttonOutlineHover: lightTheme ? "#39424E" : "#A3A6AD"
    readonly property color buttonOutlineBackgroundHover: lightTheme ? "#0D000000" : "#0DFFFFFF"
    readonly property color buttonOutlinePressed: lightTheme ? "#535B65" : "#BDC0C4"
    readonly property color buttonError: lightTheme ? "#E31B57" : "#F63D6B"
    readonly property color buttonErrorHover: lightTheme ? "#C0104A" : "#FD6F90"
    readonly property color buttonErrorPressed: lightTheme ? "#A11045" : "#FEA3B5"
    readonly property color buttonDisabled: lightTheme ? "#1A000000" : "#1A000000"
    readonly property color supportSuccess: lightTheme ? "#009B48" : "#09BF5B"
    readonly property color supportWarning: lightTheme ? "#F7A308" : "#F7A308"
    readonly property color supportError: lightTheme ? "#E31B57" : "#FD6F90"
    readonly property color supportInfo: lightTheme ? "#05BAF1" : "#0096C9"
    readonly property color selectionControl: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color notificationSuccess: lightTheme ? "#CFFCDB" : "#01532B"
    readonly property color notificationWarning: lightTheme ? "#FBF2C9" : "#8C4313"
    readonly property color notificationError: lightTheme ? "#FFE4E8" : "#891240"
    readonly property color notificationInfo: lightTheme ? "#DFF4FE" : "#085371"
    readonly property color interactive: lightTheme ? "#DD1405" : "#F23433"
    readonly property color indicatorPink: lightTheme ? "#F63D6B" : "#FD6F90"
    readonly property color indicatorYellow: lightTheme ? "#F7A308" : "#FDC121"
    readonly property color indicatorGreen: lightTheme ? "#09BF5B" : "#29DD74"
    readonly property color indicatorBlue: lightTheme ? "#05BAF1" : "#31D0FE"
    readonly property color indicatorIndigo: lightTheme ? "#477EF7" : "#69A3FB"
    readonly property color indicatorMagenta: lightTheme ? "#E248C2" : "#F4A8E3"
    readonly property color indicatorOrange: lightTheme ? "#FB6514" : "#FEB273"
    readonly property color toastBackground: lightTheme ? "#494A4D" : "#494A4D"

}

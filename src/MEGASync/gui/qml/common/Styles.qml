pragma Singleton
import QtQuick 2.15

QtObject {
    property bool lightTheme: true

    readonly property string fontFamily: "Inter"
    readonly property string fontStyleName: "normal"

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   Colors - Sorted like in Figma
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    readonly property color borderInteractive: lightTheme ? "#DD1405" : "#F23433"
    readonly property color borderStrong: lightTheme ? "#4D04101E" : "#4DF4F4F5"
    readonly property color borderStrongSelected: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color borderSubtle: lightTheme ? "#3304101E" : "#33F4F4F5"
    readonly property color borderSubtleSelected: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color borderDisabled: lightTheme ? "#1A04101E" : "#1AF4F4F5"
    readonly property color linkPrimary: lightTheme ? "#2C5BEB" : "#69A3FB"
    readonly property color linkInverse: lightTheme ? "#69A3FB" : "#2C5BEB"
    readonly property color linkVisited: lightTheme ? "#233783" : "#D9E8FF"
    readonly property color buttonBrand: lightTheme ? "#DD1405" : "#F23433"
    readonly property color buttonBrandHover: lightTheme ? "#B61714" : "#FB6361"
    readonly property color buttonBrandPressed: lightTheme ? "#931715" : "#FD9997"
    readonly property color buttonPrimary: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color buttonPrimaryHover: lightTheme ? "#39424E" : "#A3A6AD"
    readonly property color buttonPrimaryPressed: lightTheme ? "#535B65" : "#BDC0C4"
    readonly property color buttonOutline: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color buttonOutlineHover: lightTheme ? "#39424E" : "#A3A6AD"
    readonly property color buttonOutlineBackgroundHover: lightTheme ? "#0D04101E" : "#0DF4F4F5"
    readonly property color buttonOutlinePressed: lightTheme ? "#535B65" : "#BDC0C4"
    readonly property color buttonSecondary: lightTheme ? "#1A04101E" : "#1AF4F4F5"
    readonly property color buttonSecondaryHover: lightTheme ? "#2604101E" : "#26F4F4F5"
    readonly property color buttonSecondaryPressed: lightTheme ? "#3304101E" : "#33F4F4F5"
    readonly property color buttonError: lightTheme ? "#E31B57" : "#F63D6B"
    readonly property color buttonErrorHover: lightTheme ? "#C0104A" : "#FD6F90"
    readonly property color buttonErrorPressed: lightTheme ? "#A11045" : "#FEA3B5"
    readonly property color buttonDisabled: lightTheme ? "#0D04101E" : "#0DF4F4F5"
    readonly property color iconButton: lightTheme ? "#BF04101E" : "#BFF4F4F5"
    readonly property color iconButtonHover: lightTheme ? "#04101E" : "#F4F4F5"
    readonly property color iconButtonPressed: lightTheme ? "#8004101E" : "#80F4F4F5"
    readonly property color iconButtonPressedBackground: lightTheme ? "#0D04101E" : "#0DF4F4F5"
    readonly property color iconButtonDisabled: lightTheme ? "#1A04101E" : "#1AF4F4F5"
    readonly property color focus: lightTheme ? "#BDD9FF" : "#2647D0"
    readonly property color pageBackground: lightTheme ? "#FFFFFF" : "#18191A"
    readonly property color surface1: lightTheme ? "#FAFAFA" : "#303233"
    readonly property color surface2: lightTheme ? "#F3F4F4" : "#494A4D"
    readonly property color surface3: lightTheme ? "#D8D9DB" : "#616366"
    readonly property color backgroundInverse: lightTheme ? "#494A4D" : "#F3F4F4"
    readonly property color textPrimary: lightTheme ? "#303233" : "#F3F4F4"
    readonly property color textSecondary: lightTheme ? "#BF303233" : "#BFF3F4F4"
    readonly property color textAccent: lightTheme ? "#04101E" : "#FAFAFB"
    readonly property color textPlaceholder: lightTheme ? "#80303233" : "#80F3F4F4"
    readonly property color textInverseAccent: lightTheme ? "#FAFAFB" : "#04101E"
    readonly property color textOnColor: lightTheme ? "#FAFAFA" : "#FAFAFA"
    readonly property color textOnColorDisabled: lightTheme ? "#66FAFAFA" : "#80FAFAFA"
    readonly property color textError: lightTheme ? "#E31B57" : "#FD6F90"
    readonly property color textSuccess: lightTheme ? "#007C3E" : "#09BF5B"
    readonly property color textInfo: lightTheme ? "#0078A4" : "#05BAF1"
    readonly property color textWarning: lightTheme ? "#B55407" : "#F7A308"
    readonly property color textInverse: lightTheme ? "#FAFAFB" : "#303233"
    readonly property color textDisabled: lightTheme ? "#1A04101E" : "#1AF4F4F5"
    readonly property color iconPrimary: lightTheme ? "#303233" : "#F3F4F4"
    readonly property color iconSecondary: lightTheme ? "#BF303233" : "#BFF3F4F4"
    readonly property color iconAccent: lightTheme ? "#04101E" : "#FAFAFB"
    readonly property color iconInverseAccent: lightTheme ? "#FAFAFB" : "#04101E"
    readonly property color iconOnColor: lightTheme ? "#FAFAFA" : "#FAFAFA"
    readonly property color iconOnColorDisabled: lightTheme ? "#A9ABAD" : "#919397"
    readonly property color iconInverse: lightTheme ? "#FAFAFB" : "#303233"
    readonly property color iconPlaceholder: lightTheme ? "#80303233" : "#80F3F4F4"
    readonly property color iconDisabled: lightTheme ? "#1A04101E" : "#1AF4F4F5"
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
    readonly property color indicatorBackground: lightTheme ? "#1A000000" : "#1AFFFFFF"
    readonly property color indicatorPink: lightTheme ? "#F63D6B" : "#FD6F90"
    readonly property color indicatorYellow: lightTheme ? "#F7A308" : "#FDC121"
    readonly property color indicatorGreen: lightTheme ? "#09BF5B" : "#29DD74"
    readonly property color indicatorBlue: lightTheme ? "#05BAF1" : "#31D0FE"
    readonly property color indicatorIndigo: lightTheme ? "#477EF7" : "#69A3FB"
    readonly property color indicatorMagenta: lightTheme ? "#E248C2" : "#F4A8E3"
    readonly property color indicatorOrange: lightTheme ? "#FB6514" : "#FEB273"
    readonly property color toastBackground: lightTheme ? "#494A4D" : "#494A4D"
    readonly property color divider: lightTheme ? "#1A000000" : "#1AFFFFFF"
    readonly property color gradientRedTop: lightTheme ? "#FB6361" : "#FD9997"
    readonly property color gradientRedBottom: lightTheme ? "#F23433" : "#FB6361"
    readonly property color gradientGreyTop: lightTheme ? "#888D95" : "#A3A6AD"
    readonly property color gradientGreyBottom: lightTheme ? "#6E747D" : "#888D95"
    readonly property color gradientPinkTop: lightTheme ? "#FD6F90" : "#FEA3B5"
    readonly property color gradientPinkBottom: lightTheme ? "#F63D6B" : "#FD6F90"
    readonly property color gradientYellowTop: lightTheme ? "#FDC121" : "#FED64A"
    readonly property color gradientYellowBottom: lightTheme ? "#F7A308" : "#FDC121"
    readonly property color gradientGreenTop: lightTheme ? "#29DD74" : "#29DD74"
    readonly property color gradientGreenBottom: lightTheme ? "#09BF5B" : "#494A4D"
    readonly property color gradientIndigoTop: lightTheme ? "#69A3FB" : "#94C1FE"
    readonly property color gradientIndigoBottom: lightTheme ? "#477EF7" : "#69A3FB"
    readonly property color gradientMagentaTop: lightTheme ? "#ED73CC" : "#F4A8E3"
    readonly property color gradientMagentaBottom: lightTheme ? "#E248C2" : "#ED73CC"
    readonly property color gradientOrangeTop: lightTheme ? "#FD853A" : "#FEB273"
    readonly property color gradientOrangeBottom: lightTheme ? "#FB6514" : "#FD853A"
    readonly property color gradientBlueTop: lightTheme ? "#31D0FE" : "#7ADFFF"
    readonly property color gradientBlueBottom: lightTheme ? "#05BAF1" : "#31D0FE"
    readonly property color gradientContrastTop: lightTheme ? "#39424E" : "#F4F4F5"
    readonly property color gradientContrastBottom: lightTheme ? "#1E2936" : "#E5E6E8"

}

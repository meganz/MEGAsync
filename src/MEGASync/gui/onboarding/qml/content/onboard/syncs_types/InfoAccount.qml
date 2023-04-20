// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

// Local
import AccountInfoData 1.0
import Onboard 1.0

// C++
import Onboarding 1.0

Rectangle {
    width: parent.width
    height: 48
    color: Styles.pageBackground
    border.color: Styles.borderStrong
    border.width: 1
    radius: 8

    RowLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 24
            spacing: 8

            Custom.SvgImage {
                id: typeImage

                source: Images.shield_account_free
                sourceSize: Qt.size(16, 16)
            }

            Text {
                id: typeText

                text: ""
                Layout.alignment: Qt.AlignLeft
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 14
                font.underline: true

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        Qt.openUrlExternally(Links.pricing);
                    }
                }
            }

        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 24

            Text {
                text: OnboardingStrings.storageSpace
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 12
            }

            Text {
                id: usedStorage

                text: "0"
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.DemiBold
                font.pixelSize: 12
            }

            Text {
                text: "/"
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.ExtraLight
                font.pixelSize: 12
            }

            Text {
                id: totalStorage

                text: "0"
                font.family: "Inter"
                font.styleName: "normal"
                font.weight: Font.ExtraLight
                font.pixelSize: 12
            }

        }

    }

    AccountInfoData {
        id: accountInfo

        onAccountDetailsChanged: {
            switch(accountInfo.type)
            {
                case AccountInfoData.ACCOUNT_TYPE_FREE:
                    typeImage.source = Images.shield_account_free;
                    typeText.text = OnboardingStrings.accountTypeFree;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_PROI:
                    typeImage.source = Images.shield_account_proI;
                    typeText.text = OnboardingStrings.accountTypeProI;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_PROII:
                    typeImage.source = Images.shield_account_proII;
                    typeText.text = OnboardingStrings.accountTypeProII;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_PROIII:
                    typeImage.source = Images.shield_account_proIII;
                    typeText.text = OnboardingStrings.accountTypeProIII;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_LITE:
                    typeImage.source = Images.shield_account_lite;
                    typeText.text = OnboardingStrings.accountTypeLite;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_BUSINESS:
                    typeImage.source = "../../../../../images/Small_Business.png";
                    typeText.text = OnboardingStrings.accountTypeBusiness;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_PRO_FLEXI:
                    typeImage.source = "../../../../../images/Small_Pro_Flexi.png";
                    typeText.text = OnboardingStrings.accountTypeProFlexi;
                    break;
                default:
                    break;
            }

            totalStorage.text = accountInfo.totalStorage;
            usedStorage.text = accountInfo.usedStorage;
        }
    }

}

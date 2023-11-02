// System
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import AccountInfoData 1.0
import Onboard 1.0

// C++
import Onboarding 1.0

Item {
    width: parent.width
    height: 48

    Component.onCompleted: {
        AccountInfoData.requestAccountInfoData();
    }

    function getAccountTypeImage() {
        switch(AccountInfoData.type) {
            case AccountInfoData.ACCOUNT_TYPE_FREE:
                return Images.shield_account_free;
            case AccountInfoData.ACCOUNT_TYPE_PROI:
                return Images.shield_account_proI;
            case AccountInfoData.ACCOUNT_TYPE_PROII:
                return Images.shield_account_proII;
            case AccountInfoData.ACCOUNT_TYPE_PROIII:
                return Images.shield_account_proIII;
            case AccountInfoData.ACCOUNT_TYPE_LITE:
                return Images.shield_account_lite;
            case AccountInfoData.ACCOUNT_TYPE_BUSINESS:
                return Images.building;
            case AccountInfoData.ACCOUNT_TYPE_PRO_FLEXI:
                return Images.infinity;
            default:
                return "";
        }
    }

    function getAccountTypeText() {
        switch(AccountInfoData.type) {
            case AccountInfoData.ACCOUNT_TYPE_FREE:
                return OnboardingStrings.accountTypeFree;
            case AccountInfoData.ACCOUNT_TYPE_PROI:
                return OnboardingStrings.accountTypeProI;
            case AccountInfoData.ACCOUNT_TYPE_PROII:
                return OnboardingStrings.accountTypeProII;
            case AccountInfoData.ACCOUNT_TYPE_PROIII:
                return OnboardingStrings.accountTypeProIII;
            case AccountInfoData.ACCOUNT_TYPE_LITE:
                return OnboardingStrings.accountTypeLite;
            case AccountInfoData.ACCOUNT_TYPE_BUSINESS:
                return OnboardingStrings.accountTypeBusiness;
            case AccountInfoData.ACCOUNT_TYPE_PRO_FLEXI:
                return OnboardingStrings.accountTypeProFlexi
            default:
                return "";
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        color: Styles.pageBackground
        border.color: Styles.borderDisabled
        border.width: 1
        radius: 8

        RowLayout {
            anchors.fill: parent
            spacing: 0
            visible: AccountInfoData.type !== AccountInfoData.ACCOUNT_TYPE_NOT_SET

            RowLayout {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: 24
                spacing: 8

                MegaImages.SvgImage {
                    id: typeImage

                    source: getAccountTypeImage()
                    height: 16
                    width: 16
                    sourceSize: Qt.size(width, height)
                    opacity: enabled ? 1.0 : 0.2
                }

                MegaTexts.Text {
                    id: typeText

                    Layout.alignment: Qt.AlignLeft
                    font.weight: Font.DemiBold
                    font.pixelSize: MegaTexts.Text.Size.Medium
                    text: getAccountTypeText()
                }

            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 24
                visible: AccountInfoData.type !== AccountInfoData.ACCOUNT_TYPE_BUSINESS
                         && AccountInfoData.type !== AccountInfoData.ACCOUNT_TYPE_PRO_FLEXI
                         && AccountInfoData.type !== AccountInfoData.ACCOUNT_TYPE_NOT_SET

                MegaTexts.Text {
                    text: AccountInfoData.belowMinThreshold
                          ? OnboardingStrings.availableStorage
                          : OnboardingStrings.storageSpace
                    font.weight: Font.DemiBold
                }

                MegaTexts.Text {
                    font.weight: Font.DemiBold
                    text: AccountInfoData.usedStorage
                    visible: !AccountInfoData.belowMinThreshold
                }

                MegaTexts.SecondaryText {
                    font.weight: Font.DemiBold
                    text: "/"
                    visible: !AccountInfoData.belowMinThreshold
                }

                MegaTexts.SecondaryText {
                    id: totalStorage

                    font.weight: Font.DemiBold
                    text: AccountInfoData.totalStorage
                }
            }
        }
    }

    DropShadow {
        anchors.fill: parent
        horizontalOffset: 0
        verticalOffset: 5
        radius: 5.0
        samples: 11
        cached: true
        color: "#0d000000"
        source: background
        visible: parent.enabled
    }
}

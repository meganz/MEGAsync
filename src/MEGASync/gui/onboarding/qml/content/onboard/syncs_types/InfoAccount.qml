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

    readonly property string separatorStr: "/"

    width: parent.width
    height: 48
    color: Styles.pageBackground
    border.color: enabled ? Styles.borderStrong : Styles.borderDisabled
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
                visible: false
            }

            Custom.Text {
                id: typeText

                Layout.alignment: Qt.AlignLeft
                font.weight: Font.DemiBold
                font.underline: true
                font.pixelSize: Custom.Text.Size.Medium

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

            Custom.Text {
                text: OnboardingStrings.storageSpace
                font.weight: Font.DemiBold
            }

            Custom.Text {
                id: usedStorage

                font.weight: Font.DemiBold
            }

            Custom.Text {
                id: separator

                font.weight: Font.ExtraLight
            }

            Custom.Text {
                id: totalStorage

                font.weight: Font.ExtraLight
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
                    typeImage.source = Images.building;
                    typeText.text = OnboardingStrings.accountTypeBusiness;
                    break;
                case AccountInfoData.ACCOUNT_TYPE_PRO_FLEXI:
                    typeImage.source = Images.infinity;
                    typeText.text = OnboardingStrings.accountTypeProFlexi;
                    break;
                default:
                    break;
            }

            typeImage.visible = true;
            separator.text = separatorStr;
            totalStorage.text = accountInfo.totalStorage;
            usedStorage.text = accountInfo.usedStorage;
            enabled = true;
        }

        Component.onDestruction: {
            accountInfo.aboutToBeDestroyed();
        }
    }

}

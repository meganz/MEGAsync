import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import common 1.0
import components.images 1.0
import components.buttons 1.0 as Buttons

import QmlDialog 1.0
import OfferComponent 1.0

QmlDialog {
    id: window

    readonly property int defaultIconSize: 16
    readonly property int dialogWidth: 640
    readonly property int dialogHeight: 730
    readonly property int defaultMargin: 48
    readonly property int largeSpacing: 24
    readonly property int mediumSpacing: 16
    readonly property int smallSpacing: 4
    readonly property int firstSectionHeight: 368
    readonly property int storageSectionHeight: 56
    readonly property bool isDisabled: offerComponentAccess.seconds === 0

    Connections {
        target: offerComponentAccess
        function onDataUpdated() {
            productsRepeater.model = offerComponentAccess.getPlanFeatures();
            taxText.text = OfferStrings.priceDisclaimer(
                        offerComponentAccess.localCurrencyIsBillingCurrency());
        }
    }

    width: dialogWidth   // Fixed width
    height: dialogHeight  // Fixed height
    minimumWidth: dialogWidth
    minimumHeight: dialogHeight
    maximumWidth: dialogWidth
    maximumHeight: dialogHeight


    Rectangle {
        id: backgroundRectangle

        anchors.fill: parent
        color: ColorTheme.pageBackground


        Rectangle {
            id: headerImage
            width: parent.width
            height: 200
            anchors.top: parent.top

            Image {
                anchors.fill: parent
                source: Images.offerBanner
                fillMode: Image.PreserveAspectCrop
            }
        }

        Item {
            anchors {
                top: headerImage.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                topMargin: largeSpacing
                leftMargin: defaultMargin
                rightMargin: defaultMargin
                bottomMargin: defaultMargin
            }

            Rectangle {
                id: contentSection1

                width: parent.width
                height: firstSectionHeight
                anchors.top: parent.top
                color: "transparent"

                Column {
                    anchors.fill: parent
                    spacing: mediumSpacing

                    Item {
                        width: parent.width
                        height: 58
                        Column {
                            spacing: smallSpacing
                            width: parent.width
                            Text {
                                id:planName

                                width: parent.width
                                text: OfferStrings.mega.arg(offerComponentAccess.planName)
                                font.pixelSize: 24
                                lineHeight: 30
                                lineHeightMode: Text.FixedHeight
                                font.bold: true
                                color: ColorTheme.textPrimary
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                id: discountPercentageText

                                width: parent.width
                                textFormat: Text.RichText
                                
                                text: OfferStrings.discountLabel(
                                          offerComponentAccess.discountPercentage,
                                          offerComponentAccess.discountMonths,
                                          ColorTheme.textBrand)
                                font.pixelSize: 16
                                lineHeight: 24
                                lineHeightMode: Text.FixedHeight
                                color:  ColorTheme.textPrimary
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: 92
                        Column {
                            id: columnPrice
                            width: parent.width
                            spacing: window.smallSpacing

                            Text {
                                id:oldPriceText

                                width: parent.width
                                text: offerComponentAccess.totalPrice
                                font.strikeout: true
                                font.pixelSize: 14
                                lineHeight: 20
                                lineHeightMode: Text.FixedHeight
                                color: ColorTheme.textSecondary
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                id: priceText

                                width:parent.width
                                text: offerComponentAccess.discountedPrice
                                font.weight: Font.DemiBold
                                font.family: FontStyles.poppinsFontFamily
                                font.pixelSize: 40
                                lineHeight: 44
                                lineHeightMode: Text.FixedHeight
                                color: ColorTheme.textPrimary
                                wrapMode: Text.WordWrap
                                verticalAlignment: Text.AlignVCenter
                            }
                            Text {
                                id: detailsText

                                width: parent.width
                                text: offerComponentAccess.currencyName
                                font.pixelSize: 14
                                lineHeight: 20
                                lineHeightMode: Text.FixedHeight
                                color: ColorTheme.textSecondary
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    Item {
                        id: storageSection

                        width: parent.width
                        height: window.storageSectionHeight

                        Column {
                            width: parent.width
                            spacing: 8

                            Text {
                                id: storageText

                                width: parent.width
                                text: OfferStrings.storageAmount.arg(offerComponentAccess.storage)
                                font.pixelSize: 16
                                lineHeight: 24
                                lineHeightMode: Text.FixedHeight
                                font.weight: Font.Bold
                                color: ColorTheme.textPrimary
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                id: transferText

                                width: parent.width
                                text:  OfferStrings.transferAmount.arg(offerComponentAccess.transfer);
                                font.pixelSize: 16
                                lineHeight: 24
                                lineHeightMode: Text.FixedHeight
                                font.weight: Font.Bold
                                color:  ColorTheme.textPrimary
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    Item {
                        width: parent.width
                        height: Math.max(80, (offerComponentAccess.getPlanFeatures().length * 24) + ((offerComponentAccess.getPlanFeatures().length - 1) * 4))

                        Column {
                            anchors.fill: parent
                            spacing: 4   // not specified, you can set to 0/2/4 as you like

                            Repeater {
                                id: productsRepeater

                                model: offerComponentAccess.getPlanFeatures()
                                Item {
                                    id:featureItem

                                    width: parent.width
                                    height: 24

                                    Row {
                                        id: featureRow

                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 8

                                        SvgImage {
                                            sourceSize: Qt.size(defaultIconSize, defaultIconSize)
                                            source: Images.check
                                            color: ColorTheme.iconBrand
                                        }

                                        Text {
                                            text: modelData
                                            font.pixelSize: 14
                                            color: ColorTheme.textPrimary
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                    }
                                }
                            }
                        }
                    }

                    // ROW 5: note in grey, faded
                    Item {
                        width: parent.width
                        height: 20

                        Text {
                            id: taxText
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width
                            text: OfferStrings.priceDisclaimer(
                                      offerComponentAccess.localCurrencyIsBillingCurrency())
                            font.pixelSize: 12
                            lineHeight: 18
                            lineHeightMode: Text.FixedHeight

                            color: ColorTheme.textSecondary
                            elide: Text.ElideRight
                        }
                    }

                }
            }


            Rectangle {
                id: contentSection2

                width: parent.width
                height: 66
                anchors.top: contentSection1.bottom
                anchors.topMargin: 24
                color: "transparent"

                // countdown values (make them dynamic if you want)
                property int days: 28
                property int hours: 12
                property int minutes: 1

                // Reusable unit (number + label inline)
                Component {
                    id: timeUnitDelegate
                    RowLayout {
                        spacing: 4
                        Text {
                            text: modelData.value.toString().padStart(2, "0")
                            font.pixelSize: 32
                            font.weight: Font.DemiBold
                            color: ColorTheme.textPrimary
                            Layout.alignment: Qt.AlignBaseline
                        }
                        Text {
                            Layout.alignment: Qt.AlignBaseline
                            text: modelData.label
                            font.pixelSize: 12
                            color: ColorTheme.textSecondary
                        }
                    }
                }

                Item {
                    anchors.fill: parent

                    Row {
                        id: contentRow
                        spacing: 16

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top

                        Column {
                            width: 416
                            spacing: 4

                            Text {
                                id: specialOfferText
                                width: parent.width
                                text: OfferStrings.offerEnds
                                font.pixelSize: 14
                                lineHeight: 20
                                lineHeightMode: Text.FixedHeight
                                font.weight: Font.DemiBold
                                color: ColorTheme.textPrimary
                            }

                            Row {
                                spacing: 16

                                Repeater {
                                    model: [
                                        { value: offerComponentAccess.days,    label: OfferStrings.daysLabel(offerComponentAccess.days) },
                                        { value: offerComponentAccess.hours,   label: OfferStrings.hoursLabel(offerComponentAccess.hours) },
                                        { value: offerComponentAccess.minutes, label: OfferStrings.minutesLabel(offerComponentAccess.minutes) }
                                    ]
                                    delegate: timeUnitDelegate
                                }
                            }
                        }
                    }

                    Buttons.PrimaryButton {
                        id: dealButton

                        anchors.verticalCenter: contentRow.verticalCenter
                        anchors.verticalCenterOffset:11
                        anchors.right: parent.right
                        anchors.rightMargin: -4

                        text: OfferStrings.offerButton
                        sizes: Buttons.LargeSizes{}
                        colors.background:  isDisabled ? ColorTheme.buttonDisabled : ColorTheme.buttonBrand
                        colors.pressed:     isDisabled ? ColorTheme.buttonDisabled : ColorTheme.buttonBrandPressed
                        colors.hover:       isDisabled ? ColorTheme.buttonDisabled : ColorTheme.buttonBrandHover

                        colors.text:        isDisabled ? ColorTheme.textDisabled : ColorTheme.textOnColor
                        colors.textPressed: isDisabled ? ColorTheme.textDisabled : ColorTheme.textOnColor
                        colors.textHover:   isDisabled ? ColorTheme.textDisabled : ColorTheme.textOnColor

                        onClicked: {
                            offerComponentAccess.onGrabDeal();
                            accept();
                        }
                    }

                }

            }

        }
    }
}

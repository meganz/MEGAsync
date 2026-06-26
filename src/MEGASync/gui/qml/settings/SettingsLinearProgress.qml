import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0
import components.buttons 1.0 as Buttons
import components.texts 1.0 as Texts
import components.images 1.0
import AccountStateQuickWidget 1.0

Item {
    id: root

    property int progressState: AccountStateQuickWidget.OK
    property bool bannerAlreadyShown: false
    property string centerText: ""
    property var segments: []
    property bool showLegend: true

    readonly property int defaultMargin: 12
    readonly property int tightSpacing: 2
    readonly property int smallSpacing: 4
    readonly property int compactSpacing: 8
    readonly property int defaultIconSize: 16
    readonly property real legendDotSize: 7.5
    readonly property int bannerTextPixelSize: 12
    readonly property int bannerTextLineHeight: 18
    readonly property int minVisibleSegmentWidth: 4
    
    signal bannerActionClicked()

    function hasStateBanner() {
        return !root.bannerAlreadyShown
               && (progressState === AccountStateQuickWidget.WARNING
                   || progressState === AccountStateQuickWidget.FULL)
    }

    function normalStateColorForType(type) {
        switch (type) {
        case AccountStateQuickWidget.CloudDrive:
            return ColorTheme.indicatorGreen
        case AccountStateQuickWidget.Backups:
            return ColorTheme.indicatorIndigo
        case AccountStateQuickWidget.Versions:
            return ColorTheme.supportSuccess
        case AccountStateQuickWidget.Free:
            return ColorTheme.surface3
        case AccountStateQuickWidget.RubbishBin:
            return ColorTheme.iconAccent
        case AccountStateQuickWidget.Other:
            return ColorTheme.surface3
        default:
            return ColorTheme.indicatorGreen
        }
    }

    function warningStateColorForType(type) {
        switch (type) {
        case AccountStateQuickWidget.CloudDrive:
            return ColorTheme.indicatorGreen
        case AccountStateQuickWidget.Backups:
            return ColorTheme.indicatorIndigo
        case AccountStateQuickWidget.Versions:
            return ColorTheme.supportSuccess
        case AccountStateQuickWidget.Free:
            return ColorTheme.surface3
        case AccountStateQuickWidget.RubbishBin:
            return ColorTheme.iconAccent
        case AccountStateQuickWidget.Other:
            return ColorTheme.surface3
        default:
            return ColorTheme.indicatorGreen
        }
    }

    function fullStateColorForType(type) {
        switch (type) {
        case AccountStateQuickWidget.CloudDrive:
            return ColorTheme.buttonError
        case AccountStateQuickWidget.Backups:
            return ColorTheme.buttonErrorHover
        case AccountStateQuickWidget.Versions:
            return ColorTheme.buttonErrorHover
        case AccountStateQuickWidget.Free:
            return ColorTheme.surface3
        case AccountStateQuickWidget.RubbishBin:
            return ColorTheme.buttonErrorPressed
        case AccountStateQuickWidget.Other:
            return ColorTheme.buttonErrorPressed
        default:
            return ColorTheme.buttonError
        }
    }

    function bannerBackgroundColor() {
        return progressState === AccountStateQuickWidget.FULL
               ? ColorTheme.notificationError
               : ColorTheme.notificationWarning
    }

    function bannerAccentColor() {
        return progressState === AccountStateQuickWidget.FULL ? ColorTheme.textError
                                                             : ColorTheme.textWarning
    }

    function bannerTitle() {
        return progressState === AccountStateQuickWidget.FULL
               ? SettingsStrings.yourMegaAccountIsFull
               : SettingsStrings.yourMegaAccountIsNearlyFull
    }

    function bannerDescription() {
        return progressState === AccountStateQuickWidget.FULL
               ? SettingsStrings.uploadsDisabledDescription
               : SettingsStrings.nearlyFullDescription
    }

    function segmentFillColor(segment) {
        if (!segment) {
            return ColorTheme.indicatorGreen
        }

        const type = Number(segment.type)

        if (progressState === AccountStateQuickWidget.FULL) {
            return root.fullStateColorForType(type)
        }

        if (progressState === AccountStateQuickWidget.WARNING) {
            return root.warningStateColorForType(type)
        }

        return root.normalStateColorForType(type)
    }

    function legendSegments() {
        let legend = []

        ;(root.segments || []).forEach(function(segment) {
            if (Number(segment.type) === AccountStateQuickWidget.Free) {
                return
            }

            if (Number(segment.value) > 0) {
                legend.push(segment)
            }
            ;(segment.children || []).forEach(function(childSegment) {
                if (childSegment && Number(childSegment.value) > 0) {
                    legend.push(childSegment)
                }
            })
        })

        return legend
    }

    function labelForSegmentType(type) {
        switch (type) {
        case AccountStateQuickWidget.CloudDrive:
            return SettingsStrings.cloudDriveLabel
        case AccountStateQuickWidget.Backups:
            return SettingsStrings.backupsLabel
        case AccountStateQuickWidget.Versions:
            return SettingsStrings.versionsLabel
        case AccountStateQuickWidget.RubbishBin:
            return SettingsStrings.rubbishBinLabel
        case AccountStateQuickWidget.Downloads:
            return SettingsStrings.downloadsLabel
        default:
            return ""
        }
    }

    function tooltipTextForSegment(segment) {
        if (!segment) {
            return ""
        }

        const type = Number(segment.type)
        const label = root.labelForSegmentType(type)
        const sizeText = segment.sizeText || ""

        if (sizeText.length > 0) {
            switch (type) {
            case AccountStateQuickWidget.CloudDrive:
                return SettingsStrings.cloudDriveTooltipFormat.arg(sizeText).replace("[BR]", "\n")
            case AccountStateQuickWidget.Backups:
                return SettingsStrings.backupsTooltipFormat.arg(sizeText).replace("[BR]", "\n")
            case AccountStateQuickWidget.Versions:
                return SettingsStrings.versionsTooltipFormat.arg(sizeText).replace("[BR]", "\n")
            case AccountStateQuickWidget.Free:
                return SettingsStrings.availableTooltipFormat.arg(sizeText).replace("[BR]", "\n")
            case AccountStateQuickWidget.RubbishBin:
                return SettingsStrings.rubbishBinTooltipFormat.arg(sizeText).replace("[BR]", "\n")
            case AccountStateQuickWidget.Downloads:
                return SettingsStrings.downloadsTooltipFormat.arg(sizeText).replace("[BR]", "\n")
            default:
                return label
            }
        }

        return label
    }

    clip: true
    implicitHeight: progressColumn.implicitHeight

    ColumnLayout {
        id: progressColumn

        anchors.fill: parent
        spacing: root.defaultMargin

        Item {
            id: progressBarContainer

            Layout.fillWidth: true
            implicitHeight: 24

            Rectangle {
                id: progressTrack

                anchors.fill: parent
                radius: 4
                color: ColorTheme.surface3
            }

            QuotaProgressSegments {
                anchors.fill: parent
                segments: root.segments
                segmentFillColor: root.segmentFillColor
                tooltipTextForSegment: root.tooltipTextForSegment
                shouldRoundLastSegment: true
                tightSpacing: root.tightSpacing
                segmentRadius: 4
                minVisibleSegmentWidth: root.minVisibleSegmentWidth
            }

            Texts.Text {
                id: centerTextLabel

                anchors.centerIn: parent
                visible: root.centerText.length > 0
                text: root.centerText
                color: ColorTheme.textPrimary
                font.pixelSize: Texts.Text.Size.NORMAL
                font.weight: Font.DemiBold
            }
        }

        RowLayout {
            id: legendLayout

            Layout.fillWidth: true
            visible: root.showLegend
            spacing: 24

            Repeater {
                id: legendRepeater

                model: root.legendSegments()

                RowLayout {
                    id: legendItemLayout

                    required property var modelData

                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    spacing: root.tightSpacing

                    Item {
                        id: legendItemContainer

                        implicitWidth: legendItemRow.implicitWidth
                        implicitHeight: legendItemRow.implicitHeight

                        RowLayout {
                            id: legendItemRow

                            anchors.fill: parent
                            spacing: root.tightSpacing

                            Item {
                                id: legendDotContainer

                                implicitWidth: root.defaultIconSize
                                implicitHeight: root.defaultIconSize

                                Rectangle {
                                    id: legendDot

                                    anchors.centerIn: parent
                                    width: root.legendDotSize
                                    height: root.legendDotSize
                                    radius: width / 2
                                    color: root.segmentFillColor(modelData)
                                }
                            }

                            Texts.Text {
                                id: legendText

                                text: root.labelForSegmentType(Number(modelData.type))
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                                lineHeight: 16
                                lineHeightMode: Text.FixedHeight
                                color: ColorTheme.textPrimary
                            }
                        }

                        MouseArea {
                            id: legendMouseArea

                            anchors.fill: parent
                            hoverEnabled: true

                            QuotaProgressToolTip {
                                visible: legendMouseArea.containsMouse
                                text: root.tooltipTextForSegment(legendItemLayout.modelData)
                                anchorItem: legendItemContainer
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: stateBanner

            Layout.fillWidth: true
            visible: root.hasStateBanner()
            radius: 8
            color: root.bannerBackgroundColor()
            implicitHeight: bannerRow.implicitHeight + 24

            RowLayout {
                id: bannerRow

                anchors {
                    fill: parent
                    leftMargin: root.defaultMargin
                    topMargin: root.defaultMargin
                    rightMargin: 20 - bannerActionButton.sizes.focusBorderWidth
                    bottomMargin: root.defaultMargin
                }
                spacing: root.smallSpacing

                RowLayout {
                    id: bannerContentLayout

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: root.compactSpacing

                    SvgImage {
                        id: bannerIcon

                        Layout.alignment: Qt.AlignTop
                        source: root.progressState === AccountStateQuickWidget.FULL
                                ? Images.alertCircle
                                : Images.alertTriangle
                        color: root.bannerAccentColor()
                        sourceSize: Qt.size(root.defaultIconSize, root.defaultIconSize)
                    }

                    ColumnLayout {
                        id: bannerTextLayout

                        Layout.fillWidth: true
                        spacing: root.smallSpacing

                        Texts.Text {
                            id: bannerTitleText

                            Layout.fillWidth: true
                            text: root.bannerTitle()
                            color: ColorTheme.textPrimary
                            font.pixelSize: root.bannerTextPixelSize
                            font.weight: Font.DemiBold
                            lineHeight: root.bannerTextLineHeight
                            lineHeightMode: Text.FixedHeight
                        }

                        Texts.Text {
                            id: bannerDescriptionText

                            Layout.fillWidth: true
                            text: root.bannerDescription()
                            color: ColorTheme.textPrimary
                            wrapMode: Text.WordWrap
                            font.pixelSize: root.bannerTextPixelSize
                            font.weight: Font.Normal
                            lineHeight: root.bannerTextLineHeight
                            lineHeightMode: Text.FixedHeight
                        }
                    }
                }

                Buttons.PrimaryButton {
                    id: bannerActionButton

                    Layout.alignment: Qt.AlignVCenter
                    text: SettingsStrings.buyMoreStorage
                    onClicked: root.bannerActionClicked()
                }
            }
        }
    }
}

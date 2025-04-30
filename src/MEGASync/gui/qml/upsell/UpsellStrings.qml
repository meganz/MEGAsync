pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string billedMonthly: qsTr("Billed monthly")
    readonly property string billedSaveUpText: qsTr("Save up to %1% with yearly billing")
    readonly property string billedYearly: qsTr("Billed yearly")
    readonly property string buyPlan: qsTr("Buy %1")
    readonly property string estimatedPrice: qsTr("* Estimated price in your local currency. Your account will be billed in Euros for all transactions.")
    readonly property string perMonthWithBillingCurrency: qsTr("%1 per month")
    readonly property string billedYearlyWithBillingCurrency: qsTr("%1 billed yearly")
    readonly property string perMonth: qsTr("per month")
    readonly property string pricePerMonth: qsTr("(%1 per month)")
    readonly property string recommended: qsTr("Recommended")
    readonly property string currentPlan: qsTr("Current plan")
    readonly property string storage: qsTr("%1 storage")
    readonly property string storageAlmostFullTitle: qsTr("Your MEGA cloud storage is almost full")
    readonly property string storageFullTitle: qsTr("Your MEGA cloud storage is full")
    readonly property string storageText: qsTr("Upgrade your account to get more storage quota.")
    readonly property string transfer: qsTr("%1 transfer")
    readonly property string transferQuotaExceededTitle: qsTr("Transfer quota exceeded")
    readonly property string transferQuotaExceededText: qsTr("You can’t continue downloading as you’ve used all of the transfer quota available to you.[BR][BR]Upgrade your account to get more transfer quota or you can wait for [B]%1[/B] until more free quota becomes available for you.[BR][BR][B][A]Learn more about transfer quota.[/A][/B]")
    readonly property string tryProFlexi: qsTr("Need more storage?[BR][B][A]Try Pro Flexi[/A][/B]")
    readonly property string transferQuotaExceededTextPro: qsTr("You can’t continue downloading as you’ve used all of the transfer quota available to you.[BR][BR]To get more quota, purchase another paid plan.[BR][BR][B][A]Learn more about transfer quota.[/A][/B]")

}

pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    readonly property string mega: qsTr("MEGA %1")
    readonly property string storageAmount: qsTr("%1 of storage")
    readonly property string transferAmount: qsTr("%1 of transfer")
    readonly property string currencyDisclaimer: qsTr("Estimated price in your local currency.")
    readonly property string taxDisclaimer: qsTr("Tax may apply.")
    readonly property string offerEnds: qsTr("Special offers ends in:")
    readonly property string offerButton: qsTr("Grab Deal")

    function daysLabel(n) {
        // Get full translation "2 days", then remove the number
        return qsTr("%n day", "", n).replace(/^\d+\s*/, "")
    }
    function hoursLabel(n) {
        return qsTr("%n hour", "", n).replace(/^\d+\s*/, "")
    }
    function minutesLabel(n) {
        return qsTr("%n minute", "", n).replace(/^\d+\s*/, "")
    }

    function discountLabel(discount, months, color) {
        var str = qsTr("[B]%1% off[/B] for %n month", "", months)
            .arg(discount)

        var openTag =
            "<span style='color:" + color + "; font-weight:600;'>"
        var closeTag = "</span>"

        str = str.replace("[B]", openTag)
        str = str.replace("[/B]", closeTag)

        return str
    }

    function priceDisclaimer(localCurrencyIsBillingCurrency){
         return qsTr("*%1").arg(localCurrencyIsBillingCurrency ?
                                  taxDisclaimer
                                  : qsTr("%1 %2").arg(currencyDisclaimer).arg(taxDisclaimer))
    }
}

pragma Singleton
import QtQuick 2.15

QtObject {

    function formatNumber(value, decimalPoints) {
        let str = value.toFixed(decimalPoints);
        if (str.indexOf('.') !== -1) {
            str = str.replace(/\.?0+$/, ''); // Remove trailing zeros and decimal point if no decimals
        }
        return str;
    }

}

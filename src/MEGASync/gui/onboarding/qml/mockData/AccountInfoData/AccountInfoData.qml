import QtQuick 2.0

Item {
    enum AccountType {
        ACCOUNT_TYPE_FREE = 0,
        ACCOUNT_TYPE_PROI = 1,
        ACCOUNT_TYPE_PROII = 2,
        ACCOUNT_TYPE_PROIII = 3,
        ACCOUNT_TYPE_LITE = 4,
        ACCOUNT_TYPE_BUSINESS = 100,
        ACCOUNT_TYPE_PRO_FLEXI = 101
    }

    function aboutToBeDestroyed() {
        deleteLater();
    }

    signal accountDetailsChanged

    property int type: AccountInfoData.AccountType.ACCOUNT_TYPE_PROII
    property string usedStorage: "24.3 MB"
    property string totalStorage: "25 GB"

    Component.onCompleted: {
        // Simulate the arrival of the API request
        type = AccountInfoData.ACCOUNT_TYPE_LITE;
        usedStorage = "107.56 MB";
        totalStorage = "30 GB";
        accountDetailsChanged();
    }
}

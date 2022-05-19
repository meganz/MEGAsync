#pragma once

#include <QObject>
#include <QString>

class CommonMessages
{
public:
    static QString createPaymentReminder(const int64_t expirationTimeStamp);
    static QString createShellExtensionActionLabel(const QString& action, const int fileCount, const int folderCount);

private:
    static int computeDaysToExpiration(int64_t expirationTimeStampInSecs);
};

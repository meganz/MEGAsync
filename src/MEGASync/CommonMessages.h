#pragma once

#include <QObject>
#include <QString>

class CommonMessages
{
public:
    static QString createPaymentReminder(const int64_t expirationTimeStamp);
    static QString createShellExtensionActionLabel(const QString& action, const int fileCount, const int folderCount);

    static QString errorInvalidChars();

    static QString getExpiredProFlexiMessage();

    static QString getDefaultUploadFolderName();
    static QString getDefaultDownloadFolderName();
private:
    static int computeDaysToExpiration(int64_t expirationTimeStampInSecs);
};

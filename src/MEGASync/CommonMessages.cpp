#include "CommonMessages.h"

#include <QCoreApplication>
#include <QDateTime>

QString CommonMessages::createPaymentReminder(int64_t expirationTimeStamp)
{
    const int daysExpired = computeDaysToExpiration(expirationTimeStamp);
    if (daysExpired > 0)
    {
        return QCoreApplication::translate("CommonMessages","Your PRO membership plan will expire in %n day", "", daysExpired);
    }
    else if (daysExpired == 0)
    {
        return QCoreApplication::translate("CommonMessages","PRO membership plan expiring soon");
    }
    else
    {
        return QCoreApplication::translate("CommonMessages","Your PRO membership plan expired %n day ago", "", -daysExpired);
    }
}

QString CommonMessages::createShellExtensionActionLabel(const QString &action, const int fileCount, const int folderCount)
{
    QString sNumFiles = QCoreApplication::translate("ShellExtension", "%n file", "", fileCount);
    QString sNumFolders = QCoreApplication::translate("ShellExtension", "%n folder", "", folderCount);

    QString label;
    if (fileCount && folderCount)
    {
        label = QCoreApplication::translate("ShellExtension", "%1 (%2, %3)").arg(action, sNumFiles, sNumFolders);
    }
    else if (fileCount && !folderCount)
    {
        label = QCoreApplication::translate("ShellExtension", "%1 (%2)").arg(action, sNumFiles);
    }
    else if (!fileCount && folderCount)
    {
        label = QCoreApplication::translate("ShellExtension", "%1 (%2)").arg(action, sNumFolders);
    }
    return label;
}

int CommonMessages::computeDaysToExpiration(int64_t expirationTimeStampInSecs)
{
    QDateTime expiredDate;
    expiredDate.setMSecsSinceEpoch(expirationTimeStampInSecs * 1000);
    QDateTime currentDate(QDateTime::currentDateTime());
    return static_cast<int>(currentDate.daysTo(expiredDate));
}

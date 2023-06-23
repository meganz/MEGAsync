#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialogWrapper.h"
#include "Preferences.h"
#include "syncs/control/SyncController.h"

#include <QQmlContext>

class Onboarding : public QMLComponent, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT

public:

    explicit Onboarding(QObject *parent = 0);
    virtual ~Onboarding();

    QUrl getQmlUrl() override;

    QString contextName() override;
    QVector<QQmlContext::PropertyPair> contextProperties() override;


    Q_INVOKABLE void addBackups(const QStringList& localPathList);
    Q_INVOKABLE void createNextBackup(const QString& renameFolder = QString::fromUtf8(""));
    Q_INVOKABLE void openPreferences(bool sync) const;
    Q_INVOKABLE void exitLoggedIn();

signals:
    void exitLoggedInFinished();
    void backupsUpdated(const QString& path, int errorCode, bool finished);
    void backupConflict(const QString& folder, const QString& name, bool isNew);

private:
    mega::MegaApi* mMegaApi;
    SyncController* mBackupController;

    // The first field contains the full path and the second contains the backup name
    QList<QPair<QString, QString>> mBackupsToDoList;

private slots:
    void onBackupAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);

};

#endif // ONBOARDING_H

#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "QmlDialogWrapper.h"

class Syncs;
class SyncsData;
class Onboarding: public QMLComponent
{
    Q_OBJECT

public:
    explicit Onboarding(QObject *parent = 0);

    QUrl getQmlUrl() override;

    Q_INVOKABLE void openPreferences(int tabIndex) const;
    Q_INVOKABLE bool deviceNameAlreadyExists(const QString& name) const;

public slots:
    void addSync(SyncInfo::SyncOrigin origin,
                 const QString& local,
                 const QString& remote = QLatin1String("/"));
    bool checkLocalSync(const QString& path);
    bool checkRemoteSync(const QString& path);
    void clearRemoteError();
    void clearLocalError();

signals:
    void accountBlocked(int errorCode);
    void logout();

private:
    std::unique_ptr<Syncs> mSyncs;
    std::unique_ptr<SyncsData> mSyncsData;
};

#endif // ONBOARDING_H

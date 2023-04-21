#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "syncs/control/SyncController.h"
#include "qml/QmlDialogWrapper.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"
#include "syncs/control/SyncController.h"

class Onboarding : public QMLComponent, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum RegisterForm {
        FIRST_NAME = 0,
        LAST_NAME = 1,
        EMAIL = 2,
        PASSWORD = 3
    };
    Q_ENUM(RegisterForm)

    enum PasswordStrength{
        PASSWORD_STRENGTH_VERYWEAK = 0,
        PASSWORD_STRENGTH_WEAK = 1,
        PASSWORD_STRENGTH_MEDIUM = 2,
        PASSWORD_STRENGTH_GOOD = 3,
        PASSWORD_STRENGTH_STRONG = 4
    };
    Q_ENUM(PasswordStrength)

    explicit Onboarding(QObject *parent = 0);

    QUrl getQmlUrl() override;

    QString contextName() override;

    void onRequestStart(mega::MegaApi*,
                        mega::MegaRequest *request) override;

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest *request,
                         mega::MegaError* error) override;

    Q_INVOKABLE void onLoginClicked(const QVariantMap& data);
    Q_INVOKABLE void onRegisterClicked(const QVariantMap& data);
    Q_INVOKABLE void onTwoFARequested(const QString& pin);
    Q_INVOKABLE QString convertUrlToNativeFilePath(const QUrl& urlStylePath) const;
    Q_INVOKABLE void addSync(const QString& localPath, mega::MegaHandle remoteHandle = mega::INVALID_HANDLE);
    Q_INVOKABLE void addBackups(const QStringList& localPathList);
    Q_INVOKABLE bool setDeviceName(const QString& deviceName);
    Q_INVOKABLE PasswordStrength getPasswordStrength(const QString& password);

signals:
    void twoFARequired();
    void userPassFailed();
    void twoFAFailed();
    void loginFinished();
    void notNowFinished();
    void syncSetupSucces();
    void backupsUpdated(const QString& path, int errorCode, bool finished);
    void deviceNameReady(const QString& deviceName);

public slots:
    void onNotNowClicked();
    QString getComputerName();

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<Preferences> mPreferences;
    SyncController mSyncController;
    SyncController mBackupController;
    QString mPassword;
    int mNumBackupsRequested;
    int mNumBackupsProcessed;

private slots:
    void onSyncAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);
    void onBackupAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);

};

#endif // ONBOARDING_H

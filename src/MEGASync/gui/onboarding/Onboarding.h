#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialogWrapper.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"

class SyncController;

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
    Q_INVOKABLE void addSync(const QString& localPath, const mega::MegaHandle &remoteHandle);
    Q_INVOKABLE void addBackup(const QString& localPath);

signals:
    void twoFARequired();
    void userPassFailed();
    void twoFAFailed();
    void loginFinished();
    void notNowFinished();

public slots:
    void onNotNowClicked();
    QString getComputerName();

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;

    std::shared_ptr<Preferences> mPreferences;
    std::unique_ptr<SyncController> mSyncController;
    QString mPassword;

};

#endif // ONBOARDING_H

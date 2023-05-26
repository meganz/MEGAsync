#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "syncs/control/SyncController.h"
#include "qml/QmlDialogWrapper.h"
#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"
#include "Preferences.h"

class Onboarding : public QMLComponent, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT
    Q_PROPERTY(QString email MEMBER mEmail READ getEmail NOTIFY emailChanged)

public:
    enum RegisterForm
    {
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

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest *request,
                         mega::MegaError* error) override;

    void onRequestUpdate(mega::MegaApi*, mega::MegaRequest* request) override;

    void onEvent(mega::MegaApi*, mega::MegaEvent* event) override;

    Q_INVOKABLE void onLoginClicked(const QVariantMap& data);
    Q_INVOKABLE void onRegisterClicked(const QVariantMap& data);
    Q_INVOKABLE void onTwoFARequested(const QString& pin);
    Q_INVOKABLE QString convertUrlToNativeFilePath(const QUrl& urlStylePath) const;
    Q_INVOKABLE void addSync(const QString& localPath, mega::MegaHandle remoteHandle = mega::INVALID_HANDLE);
    Q_INVOKABLE bool setDeviceName(const QString& deviceName);
    Q_INVOKABLE PasswordStrength getPasswordStrength(const QString& password);
    Q_INVOKABLE void changeRegistrationEmail(const QString& email);
    Q_INVOKABLE QString getEmail();
    Q_INVOKABLE void getComputerName();
    Q_INVOKABLE void openPreferences(bool sync) const;
    Q_INVOKABLE void exitLoggedIn();

signals:
    void twoFARequired();
    void userPassFailed();
    void twoFAFailed();
    void loginFinished();
    void registerFinished(bool success);
    void exitLoggedInFinished();
    void syncSetupSuccess();
    void deviceNameReady(const QString& deviceName);
    void accountConfirmed();
    void emailChanged(const QString& email);
    void changeRegistrationEmailFinished(bool success);
    void fetchingNodesProgress(double progress);
    void cantSync(const QString& message);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    std::shared_ptr<Preferences> mPreferences;
    SyncController* mSyncController;
    QString mPassword;
    QString mEmail;
    QString mFirstName;
    QString mLastName;

private slots:
    void onSyncAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name);

};

#endif // ONBOARDING_H

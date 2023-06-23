#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialogWrapper.h"
#include "Preferences.h"

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

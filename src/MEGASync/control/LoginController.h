#ifndef LOGINCONTROLLER_H
#define LOGINCONTROLLER_H

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaRequestListener.h"
#include "mega/bindings/qt/QTMegaGlobalListener.h"

#include <QObject>
#include <QTimer>

#include <memory>

class mega::MegaApi;
class Preferences;
class LoginController : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT
    Q_PROPERTY(QString email MEMBER mEmail READ getEmail NOTIFY emailChanged)
    Q_PROPERTY(QString password MEMBER mPassword READ getPassword NOTIFY passwordChanged)

public:
    explicit LoginController(QObject *parent = nullptr);
    virtual ~LoginController();
    Q_INVOKABLE void login(const QString& email, const QString& password);
    Q_INVOKABLE void createAccount(const QString& email, const QString& password, const QString& name, const QString& lastName);
    Q_INVOKABLE void changeRegistrationEmail(const QString& email);
    Q_INVOKABLE void login2FA(const QString& pin);
    Q_INVOKABLE QString getEmail() const;
    Q_INVOKABLE QString getPassword() const;

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
    void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest* request) override;
    void onRequestStart(mega::MegaApi *api, mega::MegaRequest *request) override;
    void accountConfirmation();

signals:
    void loginFinished(int errorCode);
    void registerFinished(bool success);
    void emailChanged();
    void passwordChanged();
    void changeRegistrationEmailFinished(bool success);
    void fetchingNodesProgress(double progress);
    void fetchingNodesFinished();
    void accountConfirmed();
    void accountCreationResumed();

protected:
    virtual void onLogin(mega::MegaRequest* request, mega::MegaError* e);
    virtual void onFetchNodesSuccess();
    void onAccountCreation(mega::MegaRequest* request, mega::MegaError* e);
    void onAccountCreationResume(mega::MegaRequest* request, mega::MegaError* e);
    void onEmailChanged(mega::MegaRequest* request, mega::MegaError* e);
    void onFetchNodes(mega::MegaRequest* request, mega::MegaError* e);

    void onLogout(mega::MegaRequest* request, mega::MegaError* e);
    void fetchNodes(const QString& email = QString());

    mega::MegaApi* megaApi() {return mMegaApi;}

    std::shared_ptr<Preferences> mPreferences;

private slots:
    void runConnectivityCheck();
    void onConnectivityCheckFinished(bool success);

private:
    long long computeExclusionSizeLimit(const long long sizeLimitValue, const int unit);
    void migrateSyncConfToSdk(const QString& email);
    void loadSyncExclusionRules(const QString& email);

    mega::MegaApi * mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    QTimer *mConnectivityTimer;
    bool mFetchingNodes;
    QString mEmail;
    QString mName;
    QString mLastName;
    QString mPassword;
};

class FastLoginController : public LoginController
{
    Q_OBJECT

public:
    explicit FastLoginController(QObject* parent =  nullptr);
    bool fastLogin();

protected:
    void onLogin(mega::MegaRequest* request, mega::MegaError* e) override;
    void onFetchNodesSuccess() override;

};

class LogoutController : public QObject, mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit LogoutController(QObject* parent =  nullptr);
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;

signals:
    void onLogoutFinished();

private:
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    mega::MegaApi * mMegaApi;
};

class AccountConfirmationListener : public QObject, mega::MegaGlobalListener
{
public:
    explicit AccountConfirmationListener(LoginController* parent = nullptr);

    void onEvent(mega::MegaApi*, mega::MegaEvent* event) override;

private:
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;

};

#endif // LOGINCONTROLLER_H

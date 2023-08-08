#ifndef LOGINCONTROLLER_H
#define LOGINCONTROLLER_H

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaRequestListener.h"
#include "mega/bindings/qt/QTMegaGlobalListener.h"

#include <QObject>
#include <QTimer>

#include <memory>

class Preferences;
class LoginController : public QObject, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT
    Q_PROPERTY(QString email MEMBER mEmail READ getEmail NOTIFY emailChanged)
    Q_PROPERTY(QString password MEMBER mPassword READ getPassword)
    Q_PROPERTY(bool emailConfirmed MEMBER mEmailConfirmed READ getIsEmailConfirmed NOTIFY emailConfirmed)

public:

    explicit LoginController(QObject *parent = nullptr);
    virtual ~LoginController();
    Q_INVOKABLE void login(const QString& email, const QString& password);
    Q_INVOKABLE void createAccount(const QString& email, const QString& password, const QString& name, const QString& lastName);
    Q_INVOKABLE void changeRegistrationEmail(const QString& email);
    Q_INVOKABLE void login2FA(const QString& pin);
    Q_INVOKABLE void cancelLogin2FA();
    Q_INVOKABLE QString getEmail() const;
    Q_INVOKABLE QString getPassword() const;
    Q_INVOKABLE bool getIsEmailConfirmed() const;
    Q_INVOKABLE void cancelLogin() const;
    Q_INVOKABLE void cancelCreateAccount() const;
    Q_INVOKABLE void guestWindowLoginClicked();
    Q_INVOKABLE void guestWindowSignupClicked();
    Q_INVOKABLE bool isAccountConfirmationResumed() const;

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
    void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest* request) override;
    void onRequestStart(mega::MegaApi *api, mega::MegaRequest *request) override;

    void onEvent(mega::MegaApi*, mega::MegaEvent* event) override;

    void emailConfirmation(const QString& email);

signals:
    void loginStarted();
    void loginFinished(int errorCode, const QString& errorMsg);
    void registerStarted();
    void registerFinished(bool success);
    void emailChanged();
    void changeRegistrationEmailFinished(bool success);
    void fetchingNodesProgress(double progress);
    void fetchingNodesFinished(bool firstTime);
    void emailConfirmed();
    void accountCreationResumed();
    void logout();
    void accountCreateCancelled();
    void goToLoginPage();
    void goToSignupPage();
    void login2FACancelled();

protected:
    virtual void onLogin(mega::MegaRequest* request, mega::MegaError* e);
    virtual void onFetchNodesSuccess(bool& firstTime);
    void onAccountCreation(mega::MegaRequest* request, mega::MegaError* e);
    void onAccountCreationResume(mega::MegaRequest* request, mega::MegaError* e);
    void onEmailChanged(mega::MegaRequest* request, mega::MegaError* e);
    void onFetchNodes(mega::MegaRequest* request, mega::MegaError* e);
    void onWhyAmIBlocked(mega::MegaRequest* request, mega::MegaError* e);
    void onAccountCreationCancel(mega::MegaRequest* request, mega::MegaError* e);

    void onLogout(mega::MegaRequest* request, mega::MegaError* e);
    void fetchNodes(const QString& email = QString());

    mega::MegaApi * mMegaApi;
    std::shared_ptr<Preferences> mPreferences;

private slots:
    void runConnectivityCheck();
    void onConnectivityCheckFinished(bool success);

private:
    long long computeExclusionSizeLimit(const long long sizeLimitValue, const int unit);
    void migrateSyncConfToSdk(const QString& email);
    void loadSyncExclusionRules(const QString& email);

    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;

    QTimer *mConnectivityTimer;
    bool mFetchingNodes;
    bool mEmailConfirmed;
    bool mConfirmationResumed;
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
    void onFetchNodesSuccess(bool& firstTime) override;

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

#endif // LOGINCONTROLLER_H

#ifndef LOGIN_H
#define LOGIN_H

#include <QObject>
#include "QTMegaRequestListener.h"
#include "Preferences.h"
#include "QTMegaGlobalListener.h"

class Login : public QObject, public mega::MegaRequestListener, public mega::MegaGlobalListener
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

    Login(QObject* parent = nullptr);

    void onRequestFinish(mega::MegaApi*,
                         mega::MegaRequest *request,
                         mega::MegaError* error) override;
    void onRequestUpdate(mega::MegaApi*, mega::MegaRequest* request) override;
    void onEvent(mega::MegaApi*, mega::MegaEvent* event) override;

    Q_INVOKABLE void onLoginClicked(const QVariantMap& data);
    Q_INVOKABLE void onRegisterClicked(const QVariantMap& data);
    Q_INVOKABLE void onTwoFARequested(const QString& pin);
    Q_INVOKABLE void changeRegistrationEmail(const QString& email);
    Q_INVOKABLE QString getEmail();

signals:
    void emailChanged(const QString& email);
    void registerFinished(bool success);
    void twoFARequired();
    void userPassFailed();
    void twoFAFailed();
    void loginFinished();
    void accountConfirmed();
    void fetchingNodesProgress(double progress);
    void changeRegistrationEmailFinished(bool success);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    std::shared_ptr<Preferences> mPreferences;
    QString mPassword;
    QString mEmail;
    QString mFirstName;
    QString mLastName;
};

#endif // LOGIN_H

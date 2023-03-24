#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialogWrapper.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"

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

    Q_INVOKABLE void onTwoFACompleted(const QString& pin);

signals:
    void twoFARequired();
    void userPassFailed();
    void loginFinished();
    void notNowFinished();

public slots:
    void onForgotPasswordClicked();
    void onNotNowClicked();
    QString getComputerName();

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;

    std::shared_ptr<Preferences> mPreferences;
    QString mPassword;

};

#endif // ONBOARDING_H

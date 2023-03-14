#ifndef ONBOARDING_H
#define ONBOARDING_H

#include "qml/QmlDialog/QmlDialogWrapper.h"
#include "QTMegaRequestListener.h"
#include "Preferences.h"
#include <QDialog>


class Onboarding : public QMLComponent, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum RegisterForm{
        FIRST_NAME,
        LAST_NAME,
        EMAIL,
        PASSWORD
    };
    Q_ENUM(RegisterForm)

    explicit Onboarding(QObject *parent = 0);
    ~Onboarding();

    QUrl getQmlUrl() override;
    QString contextName() override;

    void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* error) override;

    Q_INVOKABLE void onLoginClicked(const QVariantMap& crd);
    Q_INVOKABLE void onRegisterClicked(const QVariantMap& crd);
    Q_INVOKABLE void onTwoFACompleted(const QString& pin);

signals:
    void twoFARequired();

public slots:

private:
    mega::QTMegaRequestListener *mDelegateListener;
    mega::MegaApi *mMegaApi;
    QString mEmail;
    QString mPassword;
};

#endif // ONBOARDING_H

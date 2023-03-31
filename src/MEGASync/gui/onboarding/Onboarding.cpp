#include "Preferences.h"
#include "Onboarding.h"
#include "MegaApplication.h"
#include <QQmlEngine>

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mEmail(QString())
    , mPassword(QString())
    , mPreferences(Preferences::instance())
{
    mDelegateListener = new QTMegaRequestListener(mMegaApi, this);
    mMegaApi->addRequestListener(mDelegateListener);
    qmlRegisterUncreatableType<Onboarding>("Onboarding", 1, 0, "OnboardEnum", QString::fromUtf8("Cannot create WarningLevel in QML"));
}

Onboarding::~Onboarding()
{
    delete mDelegateListener;
}

QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/main.qml"));
}

QString Onboarding::contextName()
{
    return QString::fromUtf8("Onboarding");
}

void Onboarding::onRequestStart(MegaApi *, MegaRequest *request)
{

}

void Onboarding::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *error)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_LOGIN:
        {
        qDebug()<<error->getErrorCode()<<error->getErrorString();
            if (error->getErrorCode() == MegaError::API_EFAILED
                    || error->getErrorCode() == MegaError::API_EEXPIRED)
            {
                //se oculta boton cancel?
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN Error code -> " << error->getErrorCode();
            }
            else if(error->getErrorCode() == MegaError::API_EMFAREQUIRED)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_EMFAREQUIRED";
                mEmail = QString::fromUtf8(request->getEmail());
                mPassword = QString::fromUtf8(request->getPassword());
                emit twoFARequired();
            }
            else if(error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_OK";
                mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
                auto email = request->getEmail();
                MegaSyncApp->fetchNodes(QString::fromUtf8(email ? email : ""));
                if (!mPreferences->hasLoggedIn())
                {
                    mPreferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                }
                emit loginFinished();
            }
            else
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN Error code -> " << error->getErrorCode();
                emit userPassFailed();
            }
            break;
        }
        case MegaRequest::TYPE_CREATE_ACCOUNT:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT API_OK";
                emit loginFinished(); // maybe we should change this signal
            } else {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT Error code -> " << error->getErrorCode();
            }
            break;
        }
        case MegaRequest::TYPE_MULTI_FACTOR_AUTH_CHECK:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_MULTI_FACTOR_AUTH_CHECK API_OK";
            } else {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_MULTI_FACTOR_AUTH_CHECK Error code -> " << error->getErrorCode();
            }
            break;
        }
    }
}

void Onboarding::onLoginClicked(const QVariantMap& data)
{
    qDebug() << "Onboarding::onLoginClicked" << data;

    std::string email = data.value(QString::number(EMAIL)).toString().toStdString();
    std::string password = data.value(QString::number(PASSWORD)).toString().toStdString();
    mMegaApi->login(email.c_str(), password.c_str());
}

void Onboarding::onRegisterClicked(const QVariantMap& data)
{
    qDebug() << "Onboarding::onRegisterClicked" << data;

    std::string firstName = data.value(QString::number(FIRST_NAME)).toString().toStdString();
    std::string lastName = data.value(QString::number(LAST_NAME)).toString().toStdString();
    std::string email = data.value(QString::number(EMAIL)).toString().toStdString();
    std::string password = data.value(QString::number(PASSWORD)).toString().toStdString();

    mMegaApi->createAccount(email.c_str(),
                            password.c_str(),
                            firstName.c_str(),
                            lastName.c_str());
}

void Onboarding::onForgotPasswordClicked()
{
    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#recovery")));
}

void Onboarding::onTwoFACompleted(const QString& pin)
{
    qDebug() << "Onboarding::onTwoFACompleted -> pin = " << pin;
    qDebug() << "Onboarding::onTwoFACompleted -> mEmail = " << mEmail;
    qDebug() << "Onboarding::onTwoFACompleted -> mPassword = " << mPassword;

    mMegaApi->multiFactorAuthLogin(mEmail.toUtf8().constData(),
                                   mPassword.toUtf8().constData(),
                                   pin.toUtf8().constData());
}

void Onboarding::onNotNowClicked() {
    std::unique_ptr<char[]> email(mMegaApi->getMyEmail());
    mPreferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
    emit notNowFinished();
}

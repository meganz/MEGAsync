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
{
    mDelegateListener = new QTMegaRequestListener(mMegaApi, this);
    mMegaApi->addRequestListener(mDelegateListener);
    qmlRegisterUncreatableType<Onboarding>("Onboarding", 1, 0, "OnboardEnum", QString::fromUtf8("Cannot create WarningLevel in QML"));
}

Onboarding::~Onboarding()
{
}

QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/content/App1.qml"));
}

QString Onboarding::contextName()
{
    return QString::fromUtf8("OnboardCpp");
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
        if (error->getErrorCode() == MegaError::API_EFAILED
                || error->getErrorCode() == MegaError::API_EEXPIRED)
        {
            //se oculta boton cancel?
        }
        else if(error->getErrorCode() == MegaError::API_EMFAREQUIRED)
        {
            //twoFA required
            emit twoFARequired();
        }
        else if(error->getErrorCode() == MegaError::API_OK)
        {
            auto preferences = Preferences::instance();
            preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
            auto email = request->getEmail();
            MegaSyncApp->fetchNodes(QString::fromUtf8(email ? email : ""));
            if (!preferences->hasLoggedIn())
            {
                preferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
            }
        }
    }
    }
}

void Onboarding::onLoginClicked(const QVariantMap &crd)
{
//    foreach(auto cred, crd.keys())
//    {
//        qDebug<<cred<< crd.value(cred);
//    }
    qDebug()<<crd;
    mEmail = crd.value(QString::number(EMAIL)).toString();
    mPassword = crd.value(QString::number(PASSWORD)).toString();
    mMegaApi->login(mEmail.toUtf8().constData(), mPassword.toUtf8().constData());
}

void Onboarding::onRegisterClicked(const QVariantMap &crd)
{
    QString name = crd.value(QString::number(FIRST_NAME)).toString();
    QString lastName = crd.value(QString::number(LAST_NAME)).toString();
    QString email = crd.value(QString::number(EMAIL)).toString();
    QString password = crd.value(QString::number(PASSWORD)).toString();
    qDebug()<<crd;
    mMegaApi->createAccount(email.toUtf8().constData(),
                           password.toUtf8().constData(),
                           name.toUtf8().constData(),
                            lastName.toUtf8().constData());
}

void Onboarding::onTwoFACompleted(const QString &pin)
{
    mMegaApi->multiFactorAuthLogin(mEmail.toUtf8().constData(),
                                   mPassword.toUtf8().constData(),
                                   pin.toUtf8().constData());
}



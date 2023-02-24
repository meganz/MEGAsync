#include "Preferences.h"
#include "Onboarding.h"
#include "MegaApplication.h"
#include <QQmlEngine>

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
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
        if (error->getErrorCode() == MegaError::API_EMFAREQUIRED
                || error->getErrorCode() == MegaError::API_EFAILED
                || error->getErrorCode() == MegaError::API_EEXPIRED)
        {
            //se oculta boton cancel?
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
    //mMegaApi->login(email.toUtf8().constData(), password.toUtf8().constData());
}

//void InfoWizard::on_bLogin_clicked()
//{
//    emit actionButtonClicked(LOGIN_CLICKED);
//    accept();
//}

//void InfoWizard::on_bCreateAccount_clicked()
//{
//    emit actionButtonClicked(CREATE_ACCOUNT_CLICKED);
//    accept();
//}


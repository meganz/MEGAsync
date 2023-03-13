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
            if (error->getErrorCode() == MegaError::API_EMFAREQUIRED
                    || error->getErrorCode() == MegaError::API_EFAILED
                    || error->getErrorCode() == MegaError::API_EEXPIRED)
            {
                //se oculta boton cancel?
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN Error code -> " << error->getErrorCode();
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
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_OK";
            } else {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN Error code -> " << error->getErrorCode();
            }
        }
        case MegaRequest::TYPE_CREATE_ACCOUNT:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT API_OK";
            } else {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT Error code -> " << error->getErrorCode();
            }
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


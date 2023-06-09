#include "Login.h"
#include <QDebug>
#include "MegaApplication.h"
#include <QQmlEngine>


Login::Login(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new mega::QTMegaRequestListener(mMegaApi, this))
    , mGlobalListener(new mega::QTMegaGlobalListener(mMegaApi, this))
    , mPassword(QString())
    , mLastName(QString())
    , mFirstName(QString())
    , mEmail(QString())
    , mPreferences(Preferences::instance())
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

void Login::changeRegistrationEmail(const QString &email)
{
    QString fullName = mFirstName + QString::fromUtf8(" ") + mLastName;
    mMegaApi->sendSignupLink(email.toUtf8().constData(), fullName.toUtf8().constData(), mPassword.toUtf8().constData());
}

QString Login::getEmail()
{
    return mEmail;
}

void Login::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *error)
{
    switch(request->getType())
    {
    case mega::MegaRequest::TYPE_LOGIN:
    {
        if (error->getErrorCode() == mega::MegaError::API_OK)
        {
            if(mPreferences->logged())
            {
                return;
            }
            qDebug() << "Login::onRequestFinish -> TYPE_LOGIN API_OK";
            mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
            mEmail = QString::fromUtf8(request->getEmail());
            emit emailChanged(mEmail);
            mPreferences->setEmailAndGeneralSettings(mEmail);
            if (!mPreferences->hasLoggedIn())
            {
                mPreferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
            }
            emit loginFinished();
        }
        else
        {
            mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
            if (error->getErrorCode() == mega::MegaError::API_EMFAREQUIRED)
            {
                qDebug() << "Login::onRequestFinish -> TYPE_LOGIN API_EMFAREQUIRED";
                mEmail = QString::fromUtf8(request->getEmail());
                mPassword = QString::fromUtf8(request->getPassword());
                emit twoFARequired();
            }
            else if (error->getErrorCode() == mega::MegaError::API_EFAILED)
            {
                qDebug() << "Login::onRequestFinish -> API_EFAILED";
                emit twoFAFailed();
            }
            else
            {
                qDebug() << "Login::onRequestFinish -> TYPE_LOGIN Error code -> "
                         << error->getErrorCode() << " Error string -> " << error->getErrorString();
                emit userPassFailed();
            }
        }
        break;
    }
    case mega::MegaRequest::TYPE_FETCH_NODES:
    {
        if (error->getErrorCode() == mega::MegaError::API_OK)
        {
            qDebug() << QString::fromUtf8("Login::onRequestFinish -> TYPE_CREATE_ACCOUNT Error code: %1").arg(error->getErrorCode());
            emit fetchingNodesProgress(1);
        }
        break;
    }
    case mega::MegaRequest::TYPE_CREATE_ACCOUNT:
    {
        if(error->getErrorCode() == mega::MegaError::API_OK)
        {
            mEmail = QString::fromUtf8(request->getEmail());
            mPassword = QString::fromUtf8(request->getPassword());
            mFirstName = QString::fromUtf8(request->getName());
            mLastName = QString::fromUtf8(request->getText());
            emit emailChanged(mEmail);
        }
        emit registerFinished(error->getErrorCode() == mega::MegaError::API_OK); //only fail is email already exist
        qDebug() << QString::fromUtf8("Login::onRequestFinish -> TYPE_CREATE_ACCOUNT Error code: %1").arg(error->getErrorCode());
        break;
    }
    case mega::MegaRequest::TYPE_SEND_SIGNUP_LINK:
    {
        if(error->getErrorCode() == mega::MegaError::API_OK)
        {
            mEmail = QString::fromUtf8(request->getEmail());
            emit emailChanged(mEmail);
        }
        emit changeRegistrationEmailFinished(error->getErrorCode() == mega::MegaError::API_OK);
    }
    }
}

void Login::onRequestUpdate(mega::MegaApi *, mega::MegaRequest *request)
{
    if (request->getType() == mega::MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            double total = static_cast<double>(request->getTotalBytes());
            double part = static_cast<double>(request->getTransferredBytes());
            double progress = part/total;
            if(progress > 0.6)
            {
                progress = 0.6;
            }
            emit fetchingNodesProgress(progress);
        }
    }
}

void Login::onEvent(mega::MegaApi *, mega::MegaEvent *event)
{
    if(event->getType() == mega::MegaEvent::EVENT_ACCOUNT_CONFIRMATION)
    {
        emit accountConfirmed();
    }
}

void Login::onLoginClicked(const QVariantMap& data)
{
    qDebug() << "Login::onLoginClicked" << data;

    std::string email = data.value(QString::number(EMAIL)).toString().toStdString();
    std::string password = data.value(QString::number(PASSWORD)).toString().toStdString();
    mMegaApi->login(email.c_str(), password.c_str());
}

void Login::onRegisterClicked(const QVariantMap &data)
{
    qDebug() << "Login::onRegisterClicked" << data;
    QString firstName = data.value(QString::number(FIRST_NAME)).toString();
    QString lastName = data.value(QString::number(LAST_NAME)).toString();
    QString email = data.value(QString::number(EMAIL)).toString();
    QString password = data.value(QString::number(PASSWORD)).toString();

    mMegaApi->createAccount(email.toUtf8().constData(),
                            password.toUtf8().constData(),
                            firstName.toUtf8().constData(),
                            lastName.toUtf8().constData());
}

void Login::onTwoFARequested(const QString &pin)
{
    mMegaApi->multiFactorAuthLogin(mEmail.toUtf8().constData(),
                                   mPassword.toUtf8().constData(),
                                   pin.toUtf8().constData());
}

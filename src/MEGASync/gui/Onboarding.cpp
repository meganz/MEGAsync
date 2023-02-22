#include "Preferences.h"
#include "Onboarding.h"

Onboarding::Onboarding(QObject *parent) :
    QMLComponent(parent)
{
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
    return QString::fromUtf8("Onboarding");
}

void Onboarding::loginInfo(const QString& email, const QString& password)
{
    qDebug()<<QString::fromUtf8("email:%1 password:%2").arg(email).arg(password);
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


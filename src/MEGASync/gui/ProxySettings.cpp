#include "ProxySettings.h"
#include "ui_ProxySettings.h"

#include <QNetworkProxy>

#include "megaapi.h"
#include "QMegaMessageBox.h"
#include "ConnectivityChecker.h"

using namespace mega;

ProxySettings::ProxySettings(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProxySettings),
    mApp(app)
{
    ui->setupUi(this);

    proxyTestProgressDialog = NULL;
    preferences = Preferences::instance();

    ui->eProxyPort->setValidator(new QIntValidator(0, 65535, this));

#ifdef Q_OS_LINUX
    ui->rProxyAuto->hide();
#endif

    initialize();

//    connect(ui->rNoProxy, SIGNAL(clicked()), this, SLOT(onProxySettingsChanged()));
//    connect(ui->rProxyAuto, SIGNAL(clicked()), this, SLOT(onProxySettingsChanged()));
//    connect(ui->rProxyManual, SIGNAL(clicked()), this, SLOT(onProxySettingsChanged()));
//    connect(ui->eProxyUsername, SIGNAL(textChanged(QString)), this, SLOT(onProxySettingsChanged()));
//    connect(ui->eProxyPassword, SIGNAL(textChanged(QString)), this, SLOT(onProxySettingsChanged()));
//    connect(ui->eProxyServer, SIGNAL(textChanged(QString)), this, SLOT(onProxySettingsChanged()));
//    connect(ui->eProxyPort, SIGNAL(textChanged(QString)), this, SLOT(onProxySettingsChanged()));
//    connect(ui->cProxyType, SIGNAL(currentIndexChanged(int)), this, SLOT(onProxySettingsChanged()));
//    connect(ui->cProxyRequiresPassword, SIGNAL(clicked()), this, SLOT(onProxySettingsChanged()));
}

ProxySettings::~ProxySettings()
{
    delete ui;
}

void ProxySettings::on_rProxyManual_clicked()
{
    ui->cProxyType->setEnabled(true);
    ui->eProxyServer->setEnabled(true);
    ui->eProxyPort->setEnabled(true);
    ui->cProxyRequiresPassword->setEnabled(true);
    if (ui->cProxyRequiresPassword->isChecked())
    {
        ui->eProxyUsername->setEnabled(true);
        ui->eProxyPassword->setEnabled(true);
    }
    else
    {
        ui->eProxyUsername->setEnabled(false);
        ui->eProxyPassword->setEnabled(false);
    }
}

void ProxySettings::on_rProxyAuto_clicked()
{
    ui->cProxyType->setEnabled(false);
    ui->eProxyServer->setEnabled(false);
    ui->eProxyPort->setEnabled(false);
    ui->eProxyUsername->setEnabled(false);
    ui->eProxyPassword->setEnabled(false);
    ui->cProxyRequiresPassword->setEnabled(false);
}

void ProxySettings::on_rNoProxy_clicked()
{
    ui->cProxyType->setEnabled(false);
    ui->eProxyServer->setEnabled(false);
    ui->eProxyPort->setEnabled(false);
    ui->eProxyUsername->setEnabled(false);
    ui->eProxyPassword->setEnabled(false);
    ui->cProxyRequiresPassword->setEnabled(false);
}

void ProxySettings::on_cProxyRequiresPassword_clicked()
{
    if (ui->cProxyRequiresPassword->isChecked())
    {
        ui->eProxyUsername->setEnabled(true);
        ui->eProxyPassword->setEnabled(true);
    }
    else
    {
        ui->eProxyUsername->setEnabled(false);
        ui->eProxyPassword->setEnabled(false);
    }
}

void ProxySettings::initialize()
{
    //Proxies
    ui->rNoProxy->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_NONE);
    ui->rProxyAuto->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_AUTO);
    ui->rProxyManual->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_CUSTOM);
    ui->cProxyType->setCurrentIndex(preferences->proxyProtocol());
    ui->eProxyServer->setText(preferences->proxyServer());
    ui->eProxyPort->setText(QString::number(preferences->proxyPort()));

    ui->cProxyRequiresPassword->setChecked(preferences->proxyRequiresAuth());
    ui->eProxyUsername->setText(preferences->getProxyUsername());
    ui->eProxyPassword->setText(preferences->getProxyPassword());

    if (ui->rProxyManual->isChecked())
    {
        ui->cProxyType->setEnabled(true);
        ui->eProxyServer->setEnabled(true);
        ui->eProxyPort->setEnabled(true);
        ui->cProxyRequiresPassword->setEnabled(true);
    }
    else
    {
        ui->cProxyType->setEnabled(false);
        ui->eProxyServer->setEnabled(false);
        ui->eProxyPort->setEnabled(false);
        ui->cProxyRequiresPassword->setEnabled(false);
    }

    if (ui->cProxyRequiresPassword->isChecked())
    {
        ui->eProxyUsername->setEnabled(true);
        ui->eProxyPassword->setEnabled(true);
    }
    else
    {
        ui->eProxyUsername->setEnabled(false);
        ui->eProxyPassword->setEnabled(false);
    }
}

void ProxySettings::onProxyTestError()
{
    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Proxy test failed");
    if (proxyTestProgressDialog)
    {
        proxyTestProgressDialog->hide();
        delete proxyTestProgressDialog;
        proxyTestProgressDialog = NULL;
        QMegaMessageBox::critical(nullptr, tr("Error"), tr("Your proxy settings are invalid or the proxy doesn't respond"));
    }

//    shouldClose = false;

//    reject();
}

void ProxySettings::onProxyTestSuccess()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Proxy test OK");

    if (ui->rNoProxy->isChecked())
    {
        preferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    }
    else if (ui->rProxyAuto->isChecked())
    {
        preferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    }
    else if (ui->rProxyManual->isChecked())
    {
        preferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);
    }

    preferences->setProxyProtocol(ui->cProxyType->currentIndex());
    preferences->setProxyServer(ui->eProxyServer->text().trimmed());
    preferences->setProxyPort(ui->eProxyPort->text().toInt());
    preferences->setProxyRequiresAuth(ui->cProxyRequiresPassword->isChecked());
    preferences->setProxyUsername(ui->eProxyUsername->text());
    preferences->setProxyPassword(ui->eProxyPassword->text());


    if (proxyTestProgressDialog)
    {
        proxyTestProgressDialog->hide();
        delete proxyTestProgressDialog;
        proxyTestProgressDialog = NULL;
    }

    accept();
//    if (shouldClose)
//    {
//        shouldClose = false;
//        this->close();
    //    }
}

void ProxySettings::on_applyButton_clicked()
{
//    if (modifyingSettings)
//    {
//        return;
//    }
    if (!proxyTestProgressDialog && ((ui->rNoProxy->isChecked() && (preferences->proxyType() != Preferences::PROXY_TYPE_NONE))       ||
                                     (ui->rProxyAuto->isChecked() &&  (preferences->proxyType() != Preferences::PROXY_TYPE_AUTO))    ||
                                     (ui->rProxyManual->isChecked() &&  (preferences->proxyType() != Preferences::PROXY_TYPE_CUSTOM))||
                                     (preferences->proxyProtocol() != ui->cProxyType->currentIndex())                                ||
                                     (preferences->proxyServer() != ui->eProxyServer->text().trimmed())                              ||
                                     (preferences->proxyPort() != ui->eProxyPort->text().toInt())                                    ||
                                     (preferences->proxyRequiresAuth() != ui->cProxyRequiresPassword->isChecked())                   ||
                                     (preferences->getProxyUsername() != ui->eProxyUsername->text())                                 ||
                                     (preferences->getProxyPassword() != ui->eProxyPassword->text())))
    {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        if (ui->rProxyManual->isChecked())
        {
            switch(ui->cProxyType->currentIndex())
            {
            case Preferences::PROXY_PROTOCOL_SOCKS5H:
                proxy.setType(QNetworkProxy::Socks5Proxy);
                break;
            default:
                proxy.setType(QNetworkProxy::HttpProxy);
                break;
            }

            proxy.setHostName(ui->eProxyServer->text().trimmed());
            proxy.setPort(ui->eProxyPort->text().trimmed().toUShort());
            if (ui->cProxyRequiresPassword->isChecked())
            {
                proxy.setUser(ui->eProxyUsername->text());
                proxy.setPassword(ui->eProxyPassword->text());
            }
        }
        else if (ui->rProxyAuto->isChecked())
        {
            MegaProxy *proxySettings = mApp->getMegaApi()->getAutoProxySettings();
            if (proxySettings->getProxyType()==MegaProxy::PROXY_CUSTOM)
            {
                std::string sProxyURL = proxySettings->getProxyURL();
                QString proxyURL = QString::fromUtf8(sProxyURL.data());

                QStringList parts = proxyURL.split(QString::fromAscii("://"));
                if (parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
                {
                    proxy.setType(QNetworkProxy::Socks5Proxy);
                }
                else
                {
                    proxy.setType(QNetworkProxy::HttpProxy);
                }

                QStringList arguments = parts[parts.size()-1].split(QString::fromAscii(":"));
                if (arguments.size() == 2)
                {
                    proxy.setHostName(arguments[0]);
                    proxy.setPort(arguments[1].toUShort());
                }
            }
            delete proxySettings;
        }

        proxyTestProgressDialog = new MegaProgressCustomDialog(this, 0, 0);
        proxyTestProgressDialog->setWindowModality(Qt::WindowModal);
        proxyTestProgressDialog->show();

        ConnectivityChecker *connectivityChecker = new ConnectivityChecker(Preferences::PROXY_TEST_URL);
        connectivityChecker->setProxy(proxy);
        connectivityChecker->setTestString(Preferences::PROXY_TEST_SUBSTRING);
        connectivityChecker->setTimeout(Preferences::PROXY_TEST_TIMEOUT_MS);

        connect(connectivityChecker, SIGNAL(testError()), this, SLOT(onProxyTestError()));
        connect(connectivityChecker, SIGNAL(testSuccess()), this, SLOT(onProxyTestSuccess()));
        connect(connectivityChecker, SIGNAL(testFinished()), connectivityChecker, SLOT(deleteLater()));

        connectivityChecker->startCheck();
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Testing proxy settings...");
    }
    else
    {
        reject();
    }
}

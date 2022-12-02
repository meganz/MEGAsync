#include "ProxySettings.h"
#include "ui_ProxySettings.h"

#include "megaapi.h"
#include "DialogOpener.h"

#include <QNetworkProxy>
#include "QMegaMessageBox.h"

using namespace mega;

ProxySettings::ProxySettings(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::ProxySettings),
    mApp(app),
    mPreferences(Preferences::instance()),
    mConnectivityChecker(new ConnectivityChecker(Preferences::PROXY_TEST_URL)),
    mProgressDialog(nullptr)
{
    mUi->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mUi->eProxyPort->setValidator(new QIntValidator(0, std::numeric_limits<uint16_t>::max(), this));

    initialize();

    connect(mConnectivityChecker, &ConnectivityChecker::testError,
            this, &ProxySettings::onProxyTestError);
    connect(mConnectivityChecker, &ConnectivityChecker::testSuccess,
            this, &ProxySettings::onProxyTestSuccess);

    connect(mUi->rProxyManual, &QRadioButton::clicked, this, [this]{setManualMode(true);});
    connect(mUi->cProxyRequiresPassword, &QCheckBox::toggled, this, [this]{setManualMode(true);});
    connect(mUi->rNoProxy, &QRadioButton::clicked, this, [this]{setManualMode(false);});
#ifndef Q_OS_LINUX
    connect(mUi->rProxyAuto, &QRadioButton::clicked, this, [this]{setManualMode(false);});
#endif
}

ProxySettings::~ProxySettings()
{
    delete mConnectivityChecker;
    delete mUi;
}

void ProxySettings::initialize()
{
    mUi->rNoProxy->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_NONE);
#ifndef Q_OS_LINUX
    mUi->rProxyAuto->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_AUTO);
#endif
    mUi->rProxyManual->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_CUSTOM);
    mUi->cProxyType->setCurrentIndex(mPreferences->proxyProtocol());
    mUi->eProxyServer->setText(mPreferences->proxyServer());
    mUi->eProxyPort->setText(QString::number(mPreferences->proxyPort()));

    mUi->cProxyRequiresPassword->setChecked(mPreferences->proxyRequiresAuth());
    mUi->eProxyUsername->setText(mPreferences->getProxyUsername());
    mUi->eProxyPassword->setText(mPreferences->getProxyPassword());

    setManualMode(mUi->rProxyManual->isChecked());
}

void ProxySettings::setManualMode(bool enabled)
{
    mUi->cProxyType->setEnabled(enabled);
    mUi->eProxyServer->setEnabled(enabled);
    mUi->eProxyPort->setEnabled(enabled);
    mUi->cProxyRequiresPassword->setEnabled(enabled);

    if (mUi->cProxyRequiresPassword->isEnabled())
    {
        mUi->eProxyUsername->setEnabled(mUi->cProxyRequiresPassword->isChecked());
        mUi->eProxyPassword->setEnabled(mUi->cProxyRequiresPassword->isChecked());
    }
    else
    {
        mUi->eProxyUsername->setEnabled(false);
        mUi->eProxyPassword->setEnabled(false);
    }
}

void ProxySettings::onProxyTestError()
{
    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Proxy test failed");
    DialogOpener::removeDialog(mProgressDialog);
    QMegaMessageBox::critical(this, tr("Error"),
                              tr("Your proxy settings are invalid or the proxy doesn't respond"));
}

void ProxySettings::onProxyTestSuccess()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Proxy test OK");

    if (mUi->rNoProxy->isChecked())
    {
        mPreferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    }
#ifndef Q_OS_LINUX
    else if (mUi->rProxyAuto->isChecked())
    {
        mPreferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    }
#endif
    else if (mUi->rProxyManual->isChecked())
    {
        mPreferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);
    }

    mPreferences->setProxyProtocol(mUi->cProxyType->currentIndex());
    mPreferences->setProxyServer(mUi->eProxyServer->text().trimmed());
    mPreferences->setProxyPort(mUi->eProxyPort->text().toInt());
    mPreferences->setProxyRequiresAuth(mUi->cProxyRequiresPassword->isChecked());
    mPreferences->setProxyUsername(mUi->eProxyUsername->text());
    mPreferences->setProxyPassword(mUi->eProxyPassword->text());

    DialogOpener::removeDialog(mProgressDialog);

    accept();
}

void ProxySettings::on_bUpdate_clicked()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    if (mUi->rProxyManual->isChecked())
    {
        switch(mUi->cProxyType->currentIndex())
        {
        case Preferences::PROXY_PROTOCOL_SOCKS5H:
            proxy.setType(QNetworkProxy::Socks5Proxy);
            break;
        default:
            proxy.setType(QNetworkProxy::HttpProxy);
            break;
        }

        proxy.setHostName(mUi->eProxyServer->text().trimmed());
        proxy.setPort(mUi->eProxyPort->text().trimmed().toUShort());
        if (mUi->cProxyRequiresPassword->isChecked())
        {
            proxy.setUser(mUi->eProxyUsername->text());
            proxy.setPassword(mUi->eProxyPassword->text());
        }
    }
#ifndef Q_OS_LINUX
    else if (mUi->rProxyAuto->isChecked())
    {
        MegaProxy *proxySettings = mApp->getMegaApi()->getAutoProxySettings();
        if (proxySettings->getProxyType() == MegaProxy::PROXY_CUSTOM)
        {
            std::string sProxyURL = proxySettings->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList parts = proxyURL.split(QString::fromUtf8("://"));
            if (parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
            {
                proxy.setType(QNetworkProxy::Socks5Proxy);
            }
            else
            {
                proxy.setType(QNetworkProxy::HttpProxy);
            }

            QStringList arguments = parts[parts.size()-1].split(QString::fromUtf8(":"));
            if (arguments.size() == 2)
            {
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toUShort());
            }
        }
        delete proxySettings;
    }
#endif

    //Remove it, just in case
    DialogOpener::removeDialog(mProgressDialog);
    mProgressDialog = new MegaProgressCustomDialog(this);
    mProgressDialog->setWindowModality(Qt::WindowModal);
    DialogOpener::showDialog(mProgressDialog);

    mConnectivityChecker->setProxy(proxy);
    mConnectivityChecker->setTestString(Preferences::PROXY_TEST_SUBSTRING);
    mConnectivityChecker->setTimeout(Preferences::PROXY_TEST_TIMEOUT_MS);
    mConnectivityChecker->startCheck();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Testing proxy settings...");
}

void ProxySettings::on_bCancel_clicked()
{
    reject();
}

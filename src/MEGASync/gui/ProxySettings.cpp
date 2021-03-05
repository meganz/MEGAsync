#include "ProxySettings.h"
#include "ui_ProxySettings.h"

#include <QNetworkProxy>

#include "megaapi.h"
#include "QMegaMessageBox.h"

using namespace mega;

ProxySettings::ProxySettings(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::ProxySettings),
    mApp(app),
    mPreferences(Preferences::instance()),
    mConnectivityChecker(new ConnectivityChecker(Preferences::PROXY_TEST_URL)),
    mProgressDialog(new MegaProgressCustomDialog(this))
{
    mUi->setupUi(this);
    mUi->eProxyPort->setValidator(new QIntValidator(0, 65535, this));
#ifdef Q_OS_LINUX
    mUi->rProxyAuto->hide(); // Auto Proxy mode not available on Linux flavors
#endif
    mProgressDialog->setWindowModality(Qt::WindowModal);

    initialize();

    connect(mConnectivityChecker, SIGNAL(testError()), this, SLOT(onProxyTestError()));
    connect(mConnectivityChecker, SIGNAL(testSuccess()), this, SLOT(onProxyTestSuccess()));

    connect(mUi->rProxyManual, &QRadioButton::clicked, this, [this]{setManualMode(true);});
    connect(mUi->cProxyRequiresPassword, &QCheckBox::toggled, this, [this]{setManualMode(true);});
    connect(mUi->rNoProxy, &QRadioButton::clicked, this, [this]{setManualMode(false);});
    connect(mUi->rProxyAuto, &QRadioButton::clicked, this, [this]{setManualMode(false);});
}

ProxySettings::~ProxySettings()
{
    delete mConnectivityChecker;
    delete mProgressDialog;
    delete mUi;
}

void ProxySettings::initialize()
{
    mUi->rNoProxy->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_NONE);
    mUi->rProxyAuto->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_AUTO);
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
    if (mProgressDialog->isVisible())
        mProgressDialog->hide();
    QMegaMessageBox::critical(this, tr("Error"),
                              tr("Your proxy settings are invalid or the proxy doesn't respond"));

//    reject(); reconsider in Guest mode
}

void ProxySettings::onProxyTestSuccess()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Proxy test OK");

    if (mUi->rNoProxy->isChecked())
    {
        mPreferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    }
    else if (mUi->rProxyAuto->isChecked())
    {
        mPreferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    }
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


    if (mProgressDialog->isVisible())
        mProgressDialog->hide();

    accept();
}

void ProxySettings::on_applyButton_clicked()
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
    else if (mUi->rProxyAuto->isChecked())
    {
        MegaProxy *proxySettings = mApp->getMegaApi()->getAutoProxySettings();
        if (proxySettings->getProxyType() == MegaProxy::PROXY_CUSTOM)
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

    mProgressDialog->show();

    mConnectivityChecker->setProxy(proxy);
    mConnectivityChecker->setTestString(Preferences::PROXY_TEST_SUBSTRING);
    mConnectivityChecker->setTimeout(Preferences::PROXY_TEST_TIMEOUT_MS);
    mConnectivityChecker->startCheck();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Testing proxy settings...");
}

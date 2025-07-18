#include "ProxySettings.h"

#include "DialogOpener.h"
#include "megaapi.h"
#include "TextDecorator.h"
#include "ui_ProxySettings.h"

#include <QNetworkProxy>

using namespace mega;

namespace
{
Text::Bold boldDecorator;
}
ProxySettings::ProxySettings(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::ProxySettings),
    mApp(app),
    mPreferences(Preferences::instance()),
    mConnectivityChecker(new ConnectivityChecker(Preferences::PROXY_TEST_URL)),
    mProgressDialog(nullptr)
{
    mUi->setupUi(this);

    mUi->eProxyPort->setValidator(new QIntValidator(0, std::numeric_limits<uint16_t>::max(), this));

    mProxyAuto = new QRadioButton(this);
    mProxyAuto->setText(tr("Auto-detect"));
    mProxyAuto->setCursor(Qt::PointingHandCursor);
    mUi->verticalLayout->addWidget(mProxyAuto);

    initialize();

    connect(mConnectivityChecker, &ConnectivityChecker::testFinished,
            this, &ProxySettings::onProxyTestFinished);

    connect(mUi->rProxyManual, &QRadioButton::clicked, this, [this]{setManualMode(true);});
    connect(mUi->cProxyRequiresPassword, &QCheckBox::toggled, this, [this]{setManualMode(true);});
    connect(mUi->rNoProxy,
            &QRadioButton::clicked,
            this,
            [this]
            {
                setManualMode(false);
            });
    connect(mProxyAuto,
            &QRadioButton::clicked,
            this,
            [this]
            {
                setManualMode(false);
#ifdef Q_OS_LINUX
                std::unique_ptr<MegaProxy> proxySettings(
                    mApp->getMegaApi()->getAutoProxySettings());
                if (proxySettings->getProxyType() != MegaProxy::PROXY_CUSTOM)
                {
                    auto errorMessage =
                        tr("Your system doesn’t have a proxy set. To connect, set a valid "
                           "[B]http_proxy[/B] or [B]https_proxy[/B] value in your environment.");
                    boldDecorator.process(errorMessage);
                    mUi->lErrorText->setText(errorMessage);
                    mUi->wError->setVisible(true);
                }
                else
                {
                    mUi->lErrorText->setText(QString());
                    mUi->wError->setVisible(false);
                }
#endif
            });
}

ProxySettings::~ProxySettings()
{
    delete mConnectivityChecker;
    delete mUi;
}

void ProxySettings::initialize()
{
    mUi->rNoProxy->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_NONE);
    mProxyAuto->setChecked(mPreferences->proxyType() == Preferences::PROXY_TYPE_AUTO);
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
    mUi->lErrorText->setText(QString());
    mUi->wError->setVisible(false);

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

void ProxySettings::onProxyTestFinished(bool success)
{
    if(success)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Proxy test OK");

        if (mUi->rNoProxy->isChecked())
        {
            mPreferences->setProxyType(Preferences::PROXY_TYPE_NONE);
        }
        else if (mProxyAuto->isChecked())
        {
            mPreferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
        }
        else if (mUi->rProxyManual->isChecked())
        {
            mPreferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);
        }

        mPreferences->setProxyProtocol(mUi->cProxyType->currentIndex());
        mPreferences->setProxyServer(mUi->eProxyServer->text().trimmed());
        mPreferences->setProxyPort(mUi->eProxyPort->text().toUShort());
        mPreferences->setProxyRequiresAuth(mUi->cProxyRequiresPassword->isChecked());
        mPreferences->setProxyUsername(mUi->eProxyUsername->text());
        mPreferences->setProxyPassword(mUi->eProxyPassword->text());

        if(mProgressDialog)
        {
            mProgressDialog->close();
        }

        accept();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Proxy test failed");
        if(mProgressDialog)
        {
            mProgressDialog->close();
        }
        mUi->lErrorText->setText(tr("We couldn’t connect using your proxy settings. Check your "
                                    "proxy details or try a different network."));
        mUi->wError->setVisible(true);
    }
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
    else if (mProxyAuto->isChecked())
    {
        std::unique_ptr<MegaProxy> proxySettings(mApp->getMegaApi()->getAutoProxySettings());
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

            QStringList arguments = parts[parts.size() - 1].split(QString::fromUtf8(":"));
            if (arguments.size() == 2)
            {
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toUShort());
            }
        }
#ifdef Q_OS_LINUX
        else
        {
            auto errorMessage =
                tr("Your system doesn’t have a proxy set. To connect, set a valid "
                   "[B]http_proxy[/B] or [B]https_proxy[/B] value in your environment.");
            boldDecorator.process(errorMessage);
            mUi->lErrorText->setText(errorMessage);
            mUi->wError->setVisible(true);
            return;
        }
#endif
    }

    mProgressDialog = new MegaProgressCustomDialog(this);
    mProgressDialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
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

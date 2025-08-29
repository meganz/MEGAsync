#include "ServiceUrls.h"

#include "mega/types.h"
#include "megaapi.h"
#include "Preferences.h"
#include "QmlManager.h"

#include <QChar>
#include <QString>
#include <QVariant>

#include <memory>

std::shared_ptr<ServiceUrls> ServiceUrls::instance()
{
    static std::shared_ptr<ServiceUrls> serviceUrls(new ServiceUrls());
    return serviceUrls;
}

void ServiceUrls::reset(mega::MegaApi* api)
{
    mDataReady = false;
    mDataPending = false;
    if (api)
    {
        mMegaApi = api;
        mMegaListener = std::make_unique<mega::QTMegaListener>(mMegaApi, this);
        mMegaApi->addListener(mMegaListener.get());
    }
    updateWithDomainFromSdk();
    fetchData();
}

void ServiceUrls::onEvent(mega::MegaApi*, mega::MegaEvent* event)
{
    if (event->getType() == mega::MegaEvent::EVENT_MISC_FLAGS_READY)
    {
        updateWithDomainFromSdk();
        mDataReady = true;
    }
}

void ServiceUrls::onRequestFinish(mega::MegaApi* api,
                                  mega::MegaRequest* request,
                                  mega::MegaError* e)
{
    if (!mDataReady || mDataPending) // Only process if waiting for data
    {
        switch (auto requestType = request->getType())
        {
            case mega::MegaRequest::TYPE_GET_MISC_FLAGS:
            // Fallthrough
            case mega::MegaRequest::TYPE_FETCH_NODES:
            // Fallthrough
            case mega::MegaRequest::TYPE_GET_USER_DATA:
            {
                mDataPending = false;
                if (e->getErrorCode() == mega::ErrorCodes::API_OK)
                {
                    auto msg = QString::fromUtf8("Mega domain - %1").arg(getBaseUrl().toString());
                    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
                    mDataReady = true;
                    emit dataReady();
                }
                else if (!mDataReady && requestType == mega::MegaRequest::TYPE_GET_MISC_FLAGS &&
                         e->getErrorCode() == mega::ErrorCodes::API_EACCESS)
                {
                    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                       "Mega domain - mf KO, try ud or wait for fn");
                    mDataPending = true;
                    api->getUserData();
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

bool ServiceUrls::isDataReady(bool emitSignal)
{
    if (mDataReady)
    {
        if (emitSignal)
        {
            emit dataReady();
        }
    }
    else if (!mDataPending)
    {
        fetchData();
    }
    return mDataReady;
}

QUrl ServiceUrls::getSupportEmail()
{
    return {QLatin1String("mailto:support@mega.io")};
}

QUrl ServiceUrls::getBaseUrl() const
{
    QUrl url;
    url.setScheme(QLatin1String("https"));
    url.setHost(mDomains.at(mWebsiteDomainIndex));
    return url;
}

QUrl ServiceUrls::getContactUrl() const
{
    auto url = getBaseUrl();
    url.setPath(QLatin1String("/contact"));
    return url;
}

QUrl ServiceUrls::getRecoveryUrl() const
{
    auto url = getBaseUrl();
    url.setPath(QLatin1String("/recovery"));
    return url;
}

QUrl ServiceUrls::getLinkBaseUrl() const
{
    return getBaseUrl();
}

QUrl ServiceUrls::getRemoteNodeLinkUrl(const QString& handle, const QString& key) const
{
    auto url = getLinkBaseUrl();
    url.setPath(url.path() + QLatin1String("/"));
    url.setFragment(QString::fromUtf8("!%1!%2").arg(handle, key));
    return url;
}

QUrl ServiceUrls::getDesktopAppUrl()
{
    auto url = getSupportBaseUrl();
    url.setPath(QLatin1String("/desktop"));
    return url;
}

QUrl ServiceUrls::getServiceTermsUrl()
{
    auto url = getSupportBaseUrl();
    url.setPath(QLatin1String("/terms"));
    return url;
}

QUrl ServiceUrls::getServicePolicyUrl()
{
    auto url = getSupportBaseUrl();
    url.setPath(QLatin1String("/privacy"));
    return url;
}

QUrl ServiceUrls::getHelpBaseUrl()
{
    auto url = getSupportBaseUrl();
    url.setHost(QLatin1String("help.") + url.host());
    return url;
}

QUrl ServiceUrls::getDesktopAppHelpUrl()
{
    auto url = getHelpBaseUrl();
    url.setPath(QLatin1String("/installs-apps/desktop"));
    return url;
}

QUrl ServiceUrls::getSyncHelpUrl()
{
    auto url = getDesktopAppHelpUrl();
    url.setPath(url.path() + QLatin1String("/how-does-syncing-work"));
    return url;
}

QUrl ServiceUrls::getSyncFat32HelpUrl()
{
    auto url = getDesktopAppHelpUrl();
    url.setPath(url.path() + QLatin1String("/fat-fat32-exfat"));
    return url;
}

QUrl ServiceUrls::getFileVersionHistoryHelpUrl()
{
    auto url = getHelpBaseUrl();
    url.setPath(url.path() + QLatin1String("/files-folders/restore-delete/file-version-history"));
    return url;
}

QUrl ServiceUrls::getTransferQuotaHelpUrl()
{
    auto url = getHelpBaseUrl();
    url.setPath(QLatin1String("/plans-storage/space-storage/transfer-quota"));
    return url;
}

QUrl ServiceUrls::getCreateBackupHelpUrl()
{
    auto url = getDesktopAppHelpUrl();
    url.setPath(url.path() + QLatin1String("/create-backup"));
    return url;
}

QUrl ServiceUrls::getCreateSyncHelpUrl()
{
    auto url = getDesktopAppHelpUrl();
    url.setPath(url.path() + QLatin1String("/set-up-syncs"));
    return url;
}

QUrl ServiceUrls::getAcknowledgementsUrl()
{
    return {QLatin1String("https://github.com/meganz/MEGAsync/blob/master/CREDITS.md")};
}

QUrl ServiceUrls::getDefaultStagingApiUrl()
{
    return {QLatin1String("https://staging.api.mega.co.nz/")};
}

QUrl ServiceUrls::getAutoUpdateUrl()
{
    QString randomSequence;
    for (int i = 0; i < 10; i++)
    {
        randomSequence += QChar::fromLatin1(static_cast<char>('A' + (rand() % 26)));
    }

    auto url = QUrl(qEnvironmentVariable("MEGA_UPDATE_CHECK_URL"));
    if (url.isEmpty())
    {
        url = ServiceUrls::getAutoUpdateBaseUrl();

        QString platformPath;
#ifdef WIN32
#ifdef _WIN64
        platformPath = QLatin1String("/wsync64");
#else
        platformPath = QLatin1String("/wsync");
#endif
#else
#if defined(__arm64__)
        platformPath = QLatin1String("/msyncarm64");
#else
        // Using msyncv2 to serve new updates and avoid keeping loader leftovers
        platformPath = QLatin1String("/msyncv2");
#endif
#endif

        url.setPath(url.path() + platformPath + QLatin1String("/v.txt"));
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Using environment variable MEGA_UPDATE_CHECK_URL to fetch update file");
    }

    url.setQuery(randomSequence);

    return url;
}

QUrl ServiceUrls::getProxyTestUrl()
{
    return {QLatin1String("https://g.api.mega.co.nz/cs")};
}

QStringList ServiceUrls::getHttpAllowedOrigins() const
{
    QStringList urls;
    QUrl url;
    url.setScheme(QLatin1String("https"));
    foreach(auto domainIndex, QSet<ServiceDomain>({DOMAIN_CO_NZ, DOMAIN_NZ, DOMAIN_APP}))
    {
        url.setHost(mDomains.at(domainIndex));
        urls << url.toString();
    }
    urls << QLatin1String("chrome-extension://*");
    urls << QLatin1String("moz-extension://*");
    urls << QLatin1String("edge-extension://*");

    return urls;
}

QStringList ServiceUrls::getSupportedLinksUrls() const
{
    QStringList urls(getSessionTransferBaseUrl().toString());

    QUrl url;
    foreach(auto domainIndex, QSet<ServiceDomain>({DOMAIN_CO_NZ, DOMAIN_NZ, DOMAIN_APP}))
    {
        url.setHost(mDomains.at(domainIndex));
        foreach(auto scheme, QStringList({QLatin1String("http"), QLatin1String("https")}))
        {
            url.setScheme(scheme);
            urls << url.toString() + QLatin1Char('/');
        }
    }

    return urls;
}

QUrl ServiceUrls::getSessionTransferBaseUrl()
{
    return {QLatin1String("mega:")};
}

QUrl ServiceUrls::getFmUrl()
{
    auto url = getSessionTransferBaseUrl();
    url.setPath(QLatin1String("fm"));
    return url;
}

QUrl ServiceUrls::getDeviceCenterUrl()
{
    auto url = getFmUrl();
    url.setPath(url.path() + QLatin1String("/device-centre"));
    return url;
}

QUrl ServiceUrls::getOpenInMegaUrl(const QString& deviceID, const QString& nodeHandle)
{
    auto url = getDeviceCenterUrl();
    url.setPath(url.path() + QString::fromUtf8("%1/%2").arg(deviceID, nodeHandle));
    return url;
}

QUrl ServiceUrls::getNodeUrl(const QString& nodeHandle, bool versions)
{
    auto url = getFmUrl();
    if (versions)
    {
        url.setPath(url.path() + QLatin1String("/versions"));
    }
    url.setPath(url.path() + QString::fromUtf8("/%1").arg(nodeHandle));
    return url;
}

QUrl ServiceUrls::getContactsUrl()
{
    auto url = getFmUrl();
    url.setPath(url.path() + QLatin1String("/contacts"));
    return url;
}

QUrl ServiceUrls::getContactUrl(const QString& userHandle)
{
    auto url = getFmUrl();
    url.setPath(url.path() + QString::fromUtf8("/%1").arg(userHandle));
    return url;
}

QUrl ServiceUrls::getChatUrl(const QString& userHandle)
{
    auto url = getFmUrl();
    url.setPath(url.path() + QString::fromUtf8("/chat/p/%1").arg(userHandle));
    return url;
}

QUrl ServiceUrls::getAccountUrl()
{
    auto url = getFmUrl();
    url.setPath(url.path() + QLatin1String("/account"));
    return url;
}

QUrl ServiceUrls::getSessionHistoryUrl()
{
    auto url = getAccountUrl();
    url.setPath(url.path() + QLatin1String("/history"));
    return url;
}

QUrl ServiceUrls::getAccountPlanUrl()
{
    auto url = getAccountUrl();
    url.setPath(url.path() + QLatin1String("/plan"));
    return url;
}

QUrl ServiceUrls::getAccountNotificationsUrl()
{
    auto url = getAccountUrl();
    url.setPath(url.path() + QLatin1String("/notifications"));
    return url;
}

QUrl ServiceUrls::getIncomingPendingContactUrl()
{
    auto url = getFmUrl();
    url.setPath(url.path() + QLatin1String("/ipc"));
    return url;
}

QUrl ServiceUrls::getProBaseUrl()
{
    auto url = getSessionTransferBaseUrl();
    url.setPath(QLatin1String("pro"));
    return url;
}

QUrl ServiceUrls::getProUrl()
{
    auto url = getProBaseUrl();
    url.setPath(url.path() + QString::fromUtf8("/%1").arg(getProUrlParameters()));
    return url;
}

QUrl ServiceUrls::getProFlexiUrl()
{
    auto url = getProUrl();
    url.setQuery(QLatin1String("tab=flexi"));
    return url;
}

QUrl ServiceUrls::getSmallProUrl()
{
    auto url = getProUrl();
    url.setQuery(QLatin1String("tab=exc"));
    return url;
}

QUrl ServiceUrls::getRepayBaseUrl()
{
    auto url = getSessionTransferBaseUrl();
    url.setPath(QLatin1String("repay"));
    return url;
}

QUrl ServiceUrls::getRepayUrl()
{
    auto url = getRepayBaseUrl();
    url.setPath(url.path() + QString::fromUtf8("/%1").arg(getProUrlParameters()));
    return url;
}

QUrl ServiceUrls::getUpsellPlanUrl(int proLevel, int periodInMonths)
{
    QUrl url;
    auto accountTypeVar = QVariant::fromValue(proLevel);
    auto canConvert = accountTypeVar.canConvert(qMetaTypeId<Preferences::AccountType>());

    if (canConvert)
    {
        auto accountType = accountTypeVar.value<Preferences::AccountType>();
        auto isValid =
            QMetaEnum::fromType<Preferences::AccountType>().valueToKey(accountType) != nullptr;

        if (isValid)
        {
            url = getSessionTransferBaseUrl();
            url.setPath(QString::fromUtf8("propay_%1/%2")
                            .arg(accountTypeVar.toString(), getProUrlParameters()));
        }
    }

    if (url.isEmpty())
    {
        url = getProUrl();
    }

    url.setQuery(QString::fromLatin1("m=%1").arg(periodInMonths));

    return url;
}

bool ServiceUrls::isFolderLink(const QString& link) const
{
    return isLink(link, {QLatin1String("#F!"), QLatin1String("folder/")});
}

bool ServiceUrls::isSetLink(const QString& link) const
{
    return isLink(link, {QLatin1String("collection/")});
}

void ServiceUrls::baseUrlOverride(const QString& url)
{
    const QUrl newUrl(url);
    mMegaListener.reset();
    mDataReady = true;
    mDataPending = false;
    mDomains[DOMAIN_OVERRIDE] = newUrl.host();
    mWebsiteDomainIndex = DOMAIN_OVERRIDE;
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                       QString::fromUtf8("Mega domain - set to %1")
                           .arg(mDomains.at(mWebsiteDomainIndex))
                           .toUtf8()
                           .constData());
}

mega::MegaApi* ServiceUrls::mMegaApi = nullptr;

ServiceUrls::ServiceUrls():
    mDomains{QLatin1String("mega.co.nz"),
             QLatin1String("mega.nz"),
             QLatin1String("mega.app"),
             QLatin1String("mega.io"),
             QString()},
    mMegaListener(nullptr),
    mWebsiteDomainIndex(DOMAIN_NZ),
    mDataReady(false),
    mDataPending(false)
{
    qmlRegisterUncreatableType<ServiceUrls>(
        "ServiceUrls",
        1,
        0,
        "ServiceUrls",
        QLatin1String("Warning ServiceUrls: not allowed to be instantiated"));
    QmlManager::instance()->setRootContextProperty(QLatin1String("serviceUrlsAccess"), this);
}

void ServiceUrls::fetchData()
{
    // Try to get misc flags, if it fails getUserData with NO_ACCESS will be called
    if (mMegaApi)
    {
        mDataPending = true;
        mMegaApi->getMiscFlags();
    }
}

void ServiceUrls::updateWithDomainFromSdk()
{
    if (mMegaApi)
    {
        auto* siteFlag = mMegaApi->getFlag("site");
        auto newIndex = (siteFlag && siteFlag->getGroup() == 1) ? DOMAIN_APP : DOMAIN_NZ;
        if (newIndex != mWebsiteDomainIndex)
        {
            auto oldDomain = getBaseUrl().toString();
            mWebsiteDomainIndex = newIndex;
            auto newDomain = getBaseUrl().toString();
            auto msg =
                QString::fromUtf8("Mega domain - changed from %1 to %2").arg(oldDomain, newDomain);
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
        }
        delete siteFlag;
    }
}

QUrl ServiceUrls::getSupportBaseUrl()
{
    return {QLatin1String("https://mega.io")};
}

QUrl ServiceUrls::getAutoUpdateBaseUrl()
{
    return {QLatin1String("http://g.static.mega.co.nz/upd")};
}

QString ServiceUrls::getProUrlParameters()
{
    QString params;
    auto preferences = Preferences::instance();

    if (preferences && mMegaApi)
    {
        const QString userAgent =
            QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(mMegaApi->getUserAgent())));

        params = QString::fromUtf8("uao=%1").arg(userAgent);

        mega::MegaHandle aff = mega::INVALID_HANDLE;
        int affType = mega::MegaApi::AFFILIATE_TYPE_INVALID;
        long long timestampMs = 0ll;
        preferences->getLastHandleInfo(aff, affType, timestampMs);

        if (aff != mega::INVALID_HANDLE)
        {
            std::unique_ptr<char[]> base64aff(mega::MegaApi::handleToBase64(aff));
            params.append(QString::fromUtf8("/aff=%1/aff_time=%2/aff_type=%3")
                              .arg(QString::fromLatin1(base64aff.get()),
                                   QString::number(timestampMs / 1000),
                                   QString::number(affType)));
        }
    }
    return params;
}

bool ServiceUrls::isLink(const QString& link, const QStringList& paths) const
{
    foreach(auto linkUrl, getSupportedLinksUrls())
    {
        foreach(auto path, paths)
        {
            if (link.startsWith(QString(linkUrl + path)))
            {
                return true;
            }
        }
    }
    return false;
}

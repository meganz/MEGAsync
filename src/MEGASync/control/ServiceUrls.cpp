#include "ServiceUrls.h"

#include "mega/types.h"
#include "megaapi.h"
#include "Platform.h"
#include "Preferences.h"
#include "QmlManager.h"

#include <QChar>
#include <QMutexLocker>
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
    QMutexLocker locker(&mLock);
    mDataReady = false;
    mDataPending = false;
    if (api)
    {
        mMegaApi = api;
        mMegaListener = std::make_unique<mega::QTMegaListener>(mMegaApi, this);
        mMegaApi->addListener(mMegaListener.get());
        updateWithDomainFromSdk();
        locker.unlock();
        fetchData();
    }
}

void ServiceUrls::onEvent(mega::MegaApi*, mega::MegaEvent* event)
{
    if (event->getType() == mega::MegaEvent::EVENT_MISC_FLAGS_READY)
    {
        QMutexLocker locker(&mLock);
        updateWithDomainFromSdk();
        mDataReady = true;
    }
}

void ServiceUrls::onRequestFinish(mega::MegaApi* api,
                                  mega::MegaRequest* request,
                                  mega::MegaError* e)
{
    auto requestType = request->getType();
    switch (requestType)
    {
        // We need to refresh the value when logged in
        case mega::MegaRequest::TYPE_LOGIN:
        {
            if (e->getErrorCode() == mega::ErrorCodes::API_OK)
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                   "Mega domain - login: refresh with gud");
                mLock.lock();
                mDataReady = false;
                mDataPending = true;
                mLock.unlock();
                api->getUserData();
            }
            break;
        }
        // Also when logged out
        case mega::MegaRequest::TYPE_LOGOUT:
        {
            if (!(e->getErrorCode() == mega::ErrorCodes::API_EINCOMPLETE &&
                  request->getParamType() == mega::MegaError::API_ESSL))
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                   "Mega domain - logout: refresh with gmf");
                mLock.lock();
                mDataReady = false;
                mDataPending = true;
                mLock.unlock();
                api->getMiscFlags();
            }
            break;
        }
        case mega::MegaRequest::TYPE_GET_MISC_FLAGS:
        // Fallthrough
        case mega::MegaRequest::TYPE_GET_USER_DATA:
        {
            QMutexLocker locker(&mLock);
            if (!mDataReady || mDataPending) // Only process if waiting for data
            {
                mDataPending = false;
                if (e->getErrorCode() == mega::ErrorCodes::API_OK)
                {
                    auto msg =
                        QString::fromUtf8("Mega domain - using %1").arg(getBaseUrl().toString());
                    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
                    mDataReady = true;
                    locker.unlock();
                    emit dataReady();
                }
                else if (!mDataReady && requestType == mega::MegaRequest::TYPE_GET_MISC_FLAGS &&
                         e->getErrorCode() == mega::ErrorCodes::API_EACCESS)
                {
                    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                       "Mega domain - gmf KO, try gud");
                    mDataPending = true;
                    locker.unlock();
                    api->getUserData();
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

bool ServiceUrls::isDataReady(bool emitSignal)
{
    mLock.lock();
    auto isDataReady = mDataReady;
    auto isDataPending = mDataPending;
    mLock.unlock();

    if (isDataReady)
    {
        if (emitSignal)
        {
            emit dataReady();
        }
    }
    else if (!isDataPending)
    {
        fetchData();
    }
    return isDataReady;
}

QUrl ServiceUrls::getSupportEmail()
{
    return {QLatin1String("mailto:support@mega.io")};
}

QUrl ServiceUrls::getBaseUrl() const
{
    QUrl url;
    url.setScheme(QLatin1String("https"));
    QMutexLocker locker(&mLock);
    url.setHost(mDomains[mWebsiteDomainIndex]);
    locker.unlock();
    return url;
}

QUrl ServiceUrls::getContactUrl() const
{
    auto url = getBaseUrl();
    url.setPath(QLatin1String("/contact"));
    return url;
}

QUrl ServiceUrls::getRecoveryUrl(const QString& email) const
{
    auto url = getBaseUrl();
    url.setPath(QLatin1String("/recovery"));
    if (!email.isEmpty())
    {
        url.setQuery(QLatin1String("email=%1").arg(QString::fromLatin1(email.toUtf8().toBase64())));
    }
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
    auto url = QUrl(qEnvironmentVariable("MEGA_UPDATE_CHECK_URL"));
    if (url.isEmpty())
    {
        url = ServiceUrls::getAutoUpdateBaseUrl();
        url.setPath(
            url.path() +
            QString::fromUtf8("/%1/v.txt").arg(Platform::getInstance()->getArchUpdateString()));
    }
    else
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Using environment variable MEGA_UPDATE_CHECK_URL to fetch update file");
    }

    QString randomSequence;
    for (int i = 0; i < 10; i++)
    {
        randomSequence += QChar::fromLatin1(static_cast<char>('A' + (rand() % 26)));
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
        url.setHost(mDomains[domainIndex]);
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
        url.setHost(mDomains[domainIndex]);
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

QUrl ServiceUrls::getDeviceCenterUrl()
{
    auto url = getFmUrl();
    url.setPath(url.path() + QLatin1String("/device-centre"));
    return url;
}

QUrl ServiceUrls::getOpenInMegaUrl(const QString& deviceID, const QString& nodeHandle)
{
    auto url = getDeviceCenterUrl();
    url.setPath(url.path() + QString::fromUtf8("/%1/%2").arg(deviceID, nodeHandle));
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

QUrl ServiceUrls::getProUrl() const
{
    auto url = getProBaseUrl();
    url.setPath(url.path() + QString::fromUtf8("/%1").arg(getProUrlParameters()));
    return url;
}

QUrl ServiceUrls::getProFlexiUrl() const
{
    auto url = getProUrl();
    url.setQuery(QLatin1String("tab=flexi"));
    return url;
}

QUrl ServiceUrls::getSmallProUrl() const
{
    auto url = getProUrl();
    url.setQuery(QLatin1String("tab=exc"));
    return url;
}

QUrl ServiceUrls::getRepayUrl() const
{
    auto url = getRepayBaseUrl();
    url.setPath(url.path() + QString::fromUtf8("/%1").arg(getProUrlParameters()));
    return url;
}

QUrl ServiceUrls::getUpsellPlanUrl(int proLevel, int periodInMonths) const
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
    QMutexLocker locker(&mLock);
    reset();
    mMegaListener.reset();
    mDataReady = true;
    mDataPending = false;
    mDomains[DOMAIN_OVERRIDE] = newUrl.host();
    mWebsiteDomainIndex = DOMAIN_OVERRIDE;
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                       QString::fromUtf8("Mega domain - changed to  %1 (override)")
                           .arg(mDomains[mWebsiteDomainIndex])
                           .toUtf8()
                           .constData());
}

ServiceUrls::ServiceUrls():
    mLock(QMutex::Recursive),
    mMegaListener(nullptr),
    mWebsiteDomainIndex(DOMAIN_NZ),
    mDataReady(false),
    mDataPending(false)
{
    mDomains[DOMAIN_CO_NZ] = QLatin1String("mega.co.nz");
    mDomains[DOMAIN_NZ] = QLatin1String("mega.nz");
    mDomains[DOMAIN_APP] = QLatin1String("mega.app");
    mDomains[DOMAIN_IO] = QLatin1String("mega.io");
    mDomains[DOMAIN_OVERRIDE] = QString();

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
    // Try to get misc flags, if it fails with NO_ACCESS getUserData will be called
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
        const std::unique_ptr<mega::MegaFlag> siteFlag(mMegaApi->getFlag("site"));
        // The rule for this flag is:
        //     * flag == 1 -> use the .app domain
        //     * any other value -> use the .nz domain
        auto newIndex = (siteFlag && siteFlag->getGroup() == 1) ? DOMAIN_APP : DOMAIN_NZ;
        if (newIndex != mWebsiteDomainIndex)
        {
            mWebsiteDomainIndex = newIndex;
            auto msg =
                QString::fromUtf8("Mega domain - changed to %1").arg(getBaseUrl().toString());
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
        }
        locker.unlock();
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

QUrl ServiceUrls::getFmUrl()
{
    auto url = getSessionTransferBaseUrl();
    url.setPath(QLatin1String("fm"));
    return url;
}

QUrl ServiceUrls::getProBaseUrl()
{
    auto url = getSessionTransferBaseUrl();
    url.setPath(QLatin1String("pro"));
    return url;
}

QUrl ServiceUrls::getRepayBaseUrl()
{
    auto url = getSessionTransferBaseUrl();
    url.setPath(QLatin1String("repay"));
    return url;
}

QString ServiceUrls::getProUrlParameters() const
{
    QString params;
    QString userAgent;

    // Get User Agent parameter
    mLock.lock();
    if (mMegaApi)
    {
        userAgent =
            QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(mMegaApi->getUserAgent())));
        params = QString::fromUtf8("uao=%1").arg(userAgent);
    }
    mLock.unlock();

    // Get affiliate parameter
    if (auto preferences = Preferences::instance())
    {
        mega::MegaHandle aff = mega::INVALID_HANDLE;
        int affType = mega::MegaApi::AFFILIATE_TYPE_INVALID;
        long long timestampMs = 0LL;
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

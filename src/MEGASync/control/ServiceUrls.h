#ifndef SERVICEURLS_H
#define SERVICEURLS_H

#include "mega/bindings/qt/QTMegaListener.h"
#include "megaapi.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QUrl>

#include <memory>

class ServiceUrls: public QObject, public mega::MegaListener
{
    Q_OBJECT

public:
    static std::shared_ptr<ServiceUrls> instance();

    void reset(mega::MegaApi* api = nullptr);

    ServiceUrls(const ServiceUrls&) = delete;
    ServiceUrls& operator=(const ServiceUrls&) = delete;

    void onEvent(mega::MegaApi*, mega::MegaEvent* event) override;
    void onRequestFinish(mega::MegaApi* api,
                         mega::MegaRequest* request,
                         mega::MegaError* e) override;

    bool isDataReady(bool emitSignal = false);

    static QUrl getSupportEmail();

    QUrl getBaseUrl() const;
    Q_INVOKABLE QUrl getContactUrl() const;
    Q_INVOKABLE QUrl getRecoveryUrl(const QString& email = QString()) const;
    QUrl getLinkBaseUrl() const;
    QUrl getRemoteNodeLinkUrl(const QString& handle, const QString& key) const;

    static QUrl getDesktopAppUrl();
    Q_INVOKABLE static QUrl getServiceTermsUrl();
    static QUrl getServicePolicyUrl();

    static QUrl getHelpBaseUrl();
    Q_INVOKABLE static QUrl getDesktopAppHelpUrl();
    static QUrl getSyncHelpUrl();
    static QUrl getSyncFat32HelpUrl();
    static QUrl getFileVersionHistoryHelpUrl();
    static QUrl getTransferQuotaHelpUrl();
    Q_INVOKABLE static QUrl getCreateBackupHelpUrl();
    Q_INVOKABLE static QUrl getCreateSyncHelpUrl();

    static QUrl getAcknowledgementsUrl();

    static QUrl getDefaultStagingApiUrl();
    static QUrl getAutoUpdateUrl();
    static QUrl getProxyTestUrl();

    QStringList getSupportedLinksUrls() const;

    QStringList getHttpAllowedOrigins() const;

    static QUrl getSessionTransferBaseUrl();
    static QUrl getFmUrl();
    static QUrl getDeviceCenterUrl();
    static QUrl getOpenInMegaUrl(const QString& deviceID, const QString& nodeHandle);
    static QUrl getNodeUrl(const QString& nodeHandle, bool versions = false);
    static QUrl getContactsUrl();
    static QUrl getContactUrl(const QString& userHandle);
    static QUrl getChatUrl(const QString& userHandle);
    static QUrl getAccountUrl();
    static QUrl getSessionHistoryUrl();
    static QUrl getAccountPlanUrl();
    static QUrl getAccountNotificationsUrl();
    static QUrl getIncomingPendingContactUrl();

    static QUrl getProBaseUrl();
    static QUrl getProUrl();
    static QUrl getProFlexiUrl();
    static QUrl getSmallProUrl();
    static QUrl getRepayBaseUrl();
    static QUrl getRepayUrl();
    static QUrl getUpsellPlanUrl(int proLevel, int periodInMonths);

    bool isFolderLink(const QString& link) const;
    bool isSetLink(const QString& link) const;

    void baseUrlOverride(const QString& url);

signals:
    void dataReady();

protected:
    static mega::MegaApi* mMegaApi;

private:
    enum ServiceDomain
    {
        DOMAIN_CO_NZ = 0,
        DOMAIN_NZ = 1,
        DOMAIN_APP = 2,
        DOMAIN_IO = 3,
        DOMAIN_OVERRIDE = 4,
    };

    explicit ServiceUrls();
    void fetchData();
    void updateWithDomainFromSdk();

    static QUrl getSupportBaseUrl();
    static QUrl getAutoUpdateBaseUrl();

    static QString getProUrlParameters();

    bool isLink(const QString& link, const QStringList& paths) const;

    QMap<ServiceDomain, QString> mDomains;
    std::unique_ptr<mega::QTMegaListener> mMegaListener;
    ServiceDomain mWebsiteDomainIndex;
    bool mDataReady;
    bool mDataPending;
};

#endif // SERVICEURL_H

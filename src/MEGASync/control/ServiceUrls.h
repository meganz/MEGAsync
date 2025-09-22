#ifndef SERVICEURLS_H
#define SERVICEURLS_H

#include "AppState.h"
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

    Q_INVOKABLE bool isDataReady(bool emitSignal = false);

    static QUrl getSupportEmail();

    QUrl getBaseUrl() const;
    Q_INVOKABLE QUrl getRecoveryUrl(const QString& email = QString()) const;
    QUrl getLinkBaseUrl() const;
    QUrl getRemoteNodeLinkUrl(const QString& handle, const QString& key) const;
    QUrl getRemoteSetLinkUrl(const QString& handle, const QString& key) const;

    Q_INVOKABLE static QUrl getContactSupportUrl();
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

    QUrl getProUrl() const;
    QUrl getProFlexiUrl() const;
    QUrl getSmallProUrl() const;
    QUrl getRepayUrl() const;
    QUrl getUpsellPlanUrl(int proLevel, int periodInMonths) const;

    bool isFolderLink(const QString& link) const;
    bool isSetLink(const QString& link) const;

    void baseUrlOverride(const QString& url);

signals:
    void dataReady();

protected:
    mega::MegaApi* mMegaApi;

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
    static QUrl getFmUrl();
    static QUrl getProBaseUrl();
    static QUrl getRepayBaseUrl();

    QString getProUrlParameters() const;

    bool isLink(const QString& link, const QStringList& paths) const;

    QMap<ServiceDomain, QString> mDomains;
    std::unique_ptr<mega::QTMegaListener> mMegaListener;
    ServiceDomain mWebsiteDomainIndex;
    bool mDataReady;
    bool mDataPending;

private slots:
    void onAppStateChanged(AppState::AppStates oldAppState, AppState::AppStates newAppState);
};

#endif // SERVICEURL_H

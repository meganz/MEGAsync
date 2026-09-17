#ifndef ACCOUNT_STATE_H
#define ACCOUNT_STATE_H

#include "TransferQuota.h"

#include <QObject>
#include <QString>
#include <QVariantList>

// State of the account quota surface,
// extracted from AccountStateQuickWidget so it no longer depends on QQuickWidget:
// the legacy Settings dialog hosts it through that widget and the redesigned QML
// window will own it directly. QML reaches it as the `accountStateAccess`
// context property. Its enums are registered as `AccountState` and, until the
// legacy dialog is removed, under the historical `AccountStateQuickWidget` name.
class AccountState: public QObject
{
    Q_OBJECT

public:
    enum ProgressState
    {
        OK = 0,
        WARNING,
        FULL
    };
    Q_ENUM(ProgressState)

    enum UsageSegmentType
    {
        CloudDrive = 0,
        Backups,
        RubbishBin,
        Versions,
        Downloads,
        Free,
        Other
    };
    Q_ENUM(UsageSegmentType)

    Q_PROPERTY(bool showStorageCard READ showStorageCard NOTIFY showStorageCardChanged)
    Q_PROPERTY(bool showTransferCard READ showTransferCard NOTIFY showTransferCardChanged)
    Q_PROPERTY(QString storageSummary READ storageSummary NOTIFY storageSummaryChanged)
    Q_PROPERTY(QString storageFreeText READ storageFreeText NOTIFY storageFreeTextChanged)
    Q_PROPERTY(QString storageFreeTooltipText READ storageFreeTooltipText NOTIFY
                   storageFreeTooltipTextChanged)
    Q_PROPERTY(int storagePercentage READ storagePercentage NOTIFY storagePercentageChanged)
    Q_PROPERTY(int storageState READ storageState NOTIFY storageStateChanged)
    Q_PROPERTY(QVariantList storageSegments READ storageSegments NOTIFY storageSegmentsChanged)
    Q_PROPERTY(bool storageUsageOnly READ storageUsageOnly NOTIFY storageUsageOnlyChanged)
    Q_PROPERTY(QString transferSummary READ transferSummary NOTIFY transferSummaryChanged)
    Q_PROPERTY(QString transferFreeText READ transferFreeText NOTIFY transferFreeTextChanged)
    Q_PROPERTY(QString transferFreeTooltipText READ transferFreeTooltipText NOTIFY
                   transferFreeTooltipTextChanged)
    Q_PROPERTY(int transferPercentage READ transferPercentage NOTIFY transferPercentageChanged)
    Q_PROPERTY(int transferState READ transferState NOTIFY transferStateChanged)
    Q_PROPERTY(QVariantList transferSegments READ transferSegments NOTIFY transferSegmentsChanged)
    Q_PROPERTY(bool transferValueOnly READ transferValueOnly NOTIFY transferValueOnlyChanged)
    Q_PROPERTY(QString transferValueText READ transferValueText NOTIFY transferValueTextChanged)

    explicit AccountState(QObject* parent = nullptr);

    bool showStorageCard() const;
    bool showTransferCard() const;
    QString storageSummary() const;
    QString storageFreeText() const;
    QString storageFreeTooltipText() const;
    int storagePercentage() const;
    int storageState() const;
    QVariantList storageSegments() const;
    bool storageUsageOnly() const;
    QString transferSummary() const;
    QString transferFreeText() const;
    QString transferFreeTooltipText() const;
    int transferPercentage() const;
    int transferState() const;
    QVariantList transferSegments() const;
    bool transferValueOnly() const;
    QString transferValueText() const;

    void setShowStorageCard(bool showStorageCard);
    void setShowTransferCard(bool showTransferCard);
    void updateStorageData();
    void updateTransferData(QuotaState quotaState);
    void setStorageData(const QString& summary,
                        const QString& freeText,
                        int percentage,
                        ProgressState state,
                        const QVariantList& segments,
                        bool usageOnly = false);
    void setTransferData(const QString& summary,
                         const QString& freeText,
                         int percentage,
                         ProgressState state,
                         const QVariantList& segments,
                         bool valueOnly = false,
                         const QString& valueText = QString());

signals:
    void showStorageCardChanged();
    void showTransferCardChanged();
    void storageSummaryChanged();
    void storageFreeTextChanged();
    void storageFreeTooltipTextChanged();
    void storagePercentageChanged();
    void storageStateChanged();
    void storageSegmentsChanged();
    void storageUsageOnlyChanged();
    void transferSummaryChanged();
    void transferFreeTextChanged();
    void transferFreeTooltipTextChanged();
    void transferPercentageChanged();
    void transferStateChanged();
    void transferSegmentsChanged();
    void transferValueOnlyChanged();
    void transferValueTextChanged();
    void upgradeRequested();

private:
    bool mShowStorageCard = true;
    bool mShowTransferCard = true;
    QString mStorageSummary;
    QString mStorageFreeText;
    QString mStorageFreeTooltipText;
    int mStoragePercentage = 0;
    ProgressState mStorageState = OK;
    QVariantList mStorageSegments;
    bool mStorageUsageOnly = false;
    QString mTransferSummary;
    QString mTransferFreeText;
    QString mTransferFreeTooltipText;
    int mTransferPercentage = 0;
    ProgressState mTransferState = OK;
    QVariantList mTransferSegments;
    bool mTransferValueOnly = false;
    QString mTransferValueText;
};

#endif // ACCOUNT_STATE_H

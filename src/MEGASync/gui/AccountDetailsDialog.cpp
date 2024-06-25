#include "AccountDetailsDialog.h"
#include "ui_AccountDetailsDialog.h"

#include "Utilities.h"
#include "MegaApplication.h"
#include "TransferQuota.h"

#include <QStyle>

using namespace mega;

// Precision to use for progressbars (100 for %, 1000 for ppm...)
static constexpr int PRECISION{100};
static constexpr int DEFAULT_MIN_PERCENTAGE{1};

AccountDetailsDialog::AccountDetailsDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AccountDetailsDialog)
{
    // Setup UI
    mUi->setupUi(this);

    // Set progressbars precision
    mUi->pbCloudDrive->setMaximum(PRECISION);
    mUi->pbVault->setMaximum(PRECISION);
    mUi->pbRubbish->setMaximum(PRECISION);

    // Get fresh data
    refresh();

    // Disable available storage for business accounts
    if (MegaSyncApp->getMegaApi()->isBusinessAccount())
    {
        mUi->wAvailable->hide();
    }

    // Subscribe to data updates (but detach after 1 callback)
    MegaSyncApp->attachStorageObserver(*this);
    MegaSyncApp->updateUserStats(true, true, true, true, USERSTATS_STORAGECLICKED);
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    MegaSyncApp->dettachStorageObserver(*this);
    delete mUi;
}

void AccountDetailsDialog::refresh()
{
    auto preferences = Preferences::instance();
    // Get account type
    auto accType(preferences->accountType());

    // Check if we have valid data. If not, tell the UI.
    if (preferences->totalStorage() == 0)
    {
        // We don't have data, so enable loading property
        setProperty("loading", true);
    }
    else
    {
        // We have data, so disable loading property
        setProperty("loading", false);

        // ---------- Process storage usage

        // Get useful data
        auto totalStorage(preferences->totalStorage());
        auto usedStorage(preferences->usedStorage());

        if (Utilities::isBusinessAccount())
        {
            // Set unused fields to 0
            mUi->wCircularStorage->setValue(0);

            // Disable over quota and warning
            mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
            setProperty("storageState", QLatin1String("ok"));
            mUi->lTotalStorage->hide();
        }
        else
        {
            // Check storage state and set property accordingly
            switch (preferences->getStorageState())
            {
                case MegaApi::STORAGE_STATE_UNKNOWN:
                // Fallthrough
                case MegaApi::STORAGE_STATE_GREEN:
                {
                    mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
                    setProperty("storageState", QLatin1String("warning"));
                    break;
                }
                case MegaApi::STORAGE_STATE_ORANGE:
                {
                    mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_WARNING);
                    setProperty("storageState", QLatin1String("ok"));
                    break;
                }
                case MegaApi::STORAGE_STATE_PAYWALL:
                // Fallthrough
                case MegaApi::STORAGE_STATE_RED:
                {
                    mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OVER);
                    setProperty("storageState", QLatin1String("full"));
                    break;
                }
            }

            auto parts (usedStorage ?
                           std::max(Utilities::partPer(usedStorage, totalStorage),
                                    DEFAULT_MIN_PERCENTAGE)
                                   : 0);
            mUi->wCircularStorage->setValue(parts);

            QString sepTemplate = Utilities::getTranslatedSeparatorTemplate();

            QString totalStorageString = sepTemplate.arg(QLatin1String(""), Utilities::getSizeString(totalStorage));
            mUi->lTotalStorage->show();
            mUi->lTotalStorage->setText(totalStorageString);
        }

        mUi->lUsedStorage->setText(Utilities::getSizeString(usedStorage));

        long long availableStorage = totalStorage - usedStorage;
        mUi->lAvailableStorage->setText(Utilities::getSizeString(std::max(0ll, availableStorage)));

        // ---------- Process transfer usage

        // Get useful data
        auto usedTransfer(preferences->usedBandwidth());
        auto transferQuotaState(MegaSyncApp->getTransferQuotaState());

        QString usedTransferString;

        // Set UI according to state
        switch (transferQuotaState) {
            case QuotaState::OK:
            {
                setProperty("transferState", QLatin1String("ok"));
                mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OK);
                break;
            }
            case QuotaState::WARNING:
            {
                setProperty("transferState", QLatin1String("warning"));
                mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_WARNING);
                break;
            }
            case QuotaState::OVERQUOTA:
            // Fallthrough
            case QuotaState::FULL:
            {
                setProperty("transferState", QLatin1String("full"));
                mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OVER);
                break;
            }
        }

        // Set progress bar
        switch (accType)
        {
            case Preferences::ACCOUNT_TYPE_BUSINESS:
             // Fallthrough
            case Preferences::ACCOUNT_TYPE_PRO_FLEXI:
            {
                setProperty("accountType", QLatin1String("business"));
                mUi->wCircularTransfer->setTotalValueUnknown();
                usedTransferString = Utilities::getSizeString(0ULL);
                break;
            }
            case Preferences::ACCOUNT_TYPE_FREE:
            {
                setProperty("accountType", QLatin1String("free"));
                mUi->wCircularTransfer->setTotalValueUnknown(transferQuotaState != QuotaState::FULL
                                                        && transferQuotaState != QuotaState::OVERQUOTA);
                usedTransferString = Utilities::getSizeString(usedTransfer);
                break;
            }
            case Preferences::ACCOUNT_TYPE_LITE:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_STARTER:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_BASIC:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_ESSENTIAL:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_PROI:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_PROII:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_PROIII:
            {
                setProperty("accountType", QLatin1String("pro"));

                auto totalTransfer(preferences->totalBandwidth());

                auto parts (usedTransfer ?
                               std::max(Utilities::partPer(usedTransfer, totalTransfer),
                                        DEFAULT_MIN_PERCENTAGE)
                                        : 0);

                mUi->wCircularTransfer->setValue(parts);

                QString sepTemplate = Utilities::getTranslatedSeparatorTemplate();

                usedTransferString = sepTemplate.arg(Utilities::getSizeString(usedTransfer), Utilities::getSizeString(totalTransfer));
                break;
            }
        }

        mUi->lUsedTransfer->setText(usedTransferString);

        // ---------- Process detailed storage usage

        // ---- Cloud drive storage
        auto usedCloudDriveStorage = preferences->cloudDriveStorage();
        auto parts (usedCloudDriveStorage ?
                        std::max(Utilities::partPer(usedCloudDriveStorage, totalStorage, PRECISION),
                                 DEFAULT_MIN_PERCENTAGE)
                      : 0);
        mUi->pbCloudDrive->setValue(std::min(PRECISION, parts));

        mUi->lUsedCloudDrive->setText(Utilities::getSizeString(usedCloudDriveStorage));

        // ---- Vault usage
        auto usedVaultStorage = preferences->vaultStorage();
        parts = usedVaultStorage ?
                    std::max(Utilities::partPer(usedVaultStorage, totalStorage, PRECISION),
                             DEFAULT_MIN_PERCENTAGE)
                  : 0;
        mUi->pbVault->setValue(std::min(PRECISION, parts));

        // Display only if not empty. Resize dialog to adequate height.
        if (usedVaultStorage > 0)
        {
            mUi->lUsedVault->setText(Utilities::getSizeString(usedVaultStorage));
            mUi->wVault->setVisible(true);
        }
        else
        {
            mUi->wVault->setVisible(false);
        }

        // ---- Rubbish bin usage
        auto usedRubbishStorage = preferences->rubbishStorage();
        parts = usedRubbishStorage ?
                    std::max(Utilities::partPer(usedRubbishStorage, totalStorage, PRECISION),
                             DEFAULT_MIN_PERCENTAGE)
                  : 0;
        mUi->pbRubbish->setValue(std::min(PRECISION, parts));

        mUi->lUsedRubbish->setText(Utilities::getSizeString(usedRubbishStorage));

        // ---- Versions usage
        mUi->lUsedByVersions->setText(Utilities::getSizeString(preferences->versionsStorage()));

        // ---------- Refresh display
        updateGeometry();
        style()->unpolish(this);
        style()->polish(this);
        update();
    }
}

void AccountDetailsDialog::updateStorageElements()
{
    // Prevent other updates of these fields (due to events) after the first one
    MegaSyncApp->dettachStorageObserver(*this);

    refresh();
}

void AccountDetailsDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        refresh();
    }

    QDialog::changeEvent(event);
}

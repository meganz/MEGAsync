#include "AccountDetailsDialog.h"
#include "ui_AccountDetailsDialog.h"

#include "control/Utilities.h"
#include "MegaApplication.h"
#include "TransferQuota.h"

#include <QStyle>

using namespace mega;

// Precision to use for progressbars (100 for %, 1000 for ppm...)
static constexpr int PRECISION{100};

AccountDetailsDialog::AccountDetailsDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AccountDetailsDialog)
{
    // Setup UI
    mUi->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Set progressbars precision
    mUi->pbCloudDrive->setMaximum(PRECISION);
    mUi->pbInbox->setMaximum(PRECISION);
    mUi->pbRubbish->setMaximum(PRECISION);

    // Set transfer quota progress bar color to blue
    mUi->wCircularTransfer->setProgressBarGradient(QColor("#60D1FE"), QColor("#58B9F3"));

    // Get fresh data
    refresh(Preferences::instance());

    // Init HiDPI
    mHighDpiResize.init(this);

    // Subscribe to data updates (but detach after 1 callback)
    qobject_cast<MegaApplication*>(qApp)->attachStorageObserver(*this);
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    qobject_cast<MegaApplication*>(qApp)->dettachStorageObserver(*this);
    delete mUi;
}

void AccountDetailsDialog::refresh(Preferences* preferences)
{
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

        // Separator between used and total.
        QString sep(QString::fromUtf8(" / "));

        // ---------- Process storage usage

        // Check storage state and set property accordingly
        switch (preferences->getStorageState())
        {
            case MegaApi::STORAGE_STATE_UNKNOWN:
            // Fallthrough
            case MegaApi::STORAGE_STATE_GREEN:
            {
                mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
                setProperty("storageState", QString::fromUtf8("ok"));
                break;
            }
            case MegaApi::STORAGE_STATE_ORANGE:
            {
                mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_WARNING);
                setProperty("storageState", QString::fromUtf8("warning"));
                break;
            }
            case MegaApi::STORAGE_STATE_PAYWALL:
            // Fallthrough
            case MegaApi::STORAGE_STATE_RED:
            {
                mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OVER);
                setProperty("storageState", QString::fromUtf8("full"));
                break;
            }
        }

        // Get useful data
        auto totalStorage(preferences->totalStorage());
        auto usedStorage(preferences->usedStorage());

        if (accType == Preferences::ACCOUNT_TYPE_BUSINESS)
        {
            // Set unused fields to 0
            mUi->wCircularStorage->setValue(0);
            mUi->lTotalTransfer->setText(QString());

            // Disable over quota and warning
            mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
            setProperty("storageState", QString::fromUtf8("ok"));
        }
        else
        {
            mUi->wCircularStorage->setValue(Utilities::partPer(usedStorage, totalStorage));
            mUi->lTotalStorage->setText(sep + Utilities::getSizeString(totalStorage));
        }

        mUi->lUsedStorage->setText(Utilities::getSizeString(usedStorage));

        // ---------- Process transfer usage

        // Get useful data
        auto totalTransfer(preferences->totalBandwidth());
        auto usedTransfer(preferences->usedBandwidth());
        auto transferQuotaState(qobject_cast<MegaApplication*>(qApp)->getTransferQuotaState());

        // Set UI according to state
        switch (transferQuotaState) {
            case QuotaState::OK:
            {
                setProperty("transferState", QString::fromUtf8("ok"));
                break;
            }
            case QuotaState::WARNING:
            {
                setProperty("transferState", QString::fromUtf8("warning"));
                break;
            }
            case QuotaState::FULL:
            {
                setProperty("transferState", QString::fromUtf8("full"));
                break;
            }
        }

        QString totalTransferString;

        // Set progress bar
        switch (accType)
        {
            case Preferences::ACCOUNT_TYPE_BUSINESS:
            {
                setProperty("accountType", QString::fromUtf8("business"));
                mUi->wCircularStorage->setValue(0);
                break;
            }
            case Preferences::ACCOUNT_TYPE_FREE:
            {
                setProperty("accountType", QString::fromUtf8("free"));

                // Set progress bar value
                switch (transferQuotaState) {
                    case QuotaState::OK:
                    // Fallthrough
                    case QuotaState::WARNING:
                    {
                        mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OK);
                        mUi->wCircularTransfer->setEmptyBarTotalValueUnknown();
                        break;
                    }
                    case QuotaState::FULL:
                    {
                        mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OVER);
                        mUi->wCircularTransfer->setFullBarTotalValueUnkown();
                        break;
                    }
                }
                break;
            }
            case Preferences::ACCOUNT_TYPE_LITE:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_PROI:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_PROII:
            // Fallthrough
            case Preferences::ACCOUNT_TYPE_PROIII:
            {
                setProperty("accountType", QString::fromUtf8("pro"));

                // Check transfer state and set property accordingly
                switch (transferQuotaState)
                {
                    case QuotaState::OK:
                    {
                        mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OK);
                        break;
                    }
                    case QuotaState::WARNING:
                    {
                        mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_WARNING);
                        break;
                    }
                    // Fallthrough
                    case QuotaState::FULL:
                    {
                        mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OVER);
                        break;
                    }
                }

                mUi->wCircularTransfer->setValue(Utilities::partPer(usedTransfer, totalTransfer));

                // Build total transfer string
                totalTransferString = sep + Utilities::getSizeString(totalTransfer);
                break;
            }
        }
        mUi->lTotalTransfer->setText(totalTransferString);
        mUi->lUsedTransfer->setText(Utilities::getSizeString(usedTransfer));

        // ---------- Process detailed storage usage

        // Compute usage wrt used storage
        totalStorage = usedStorage;

        // ---- Cloud drive storage
        usedStorage = preferences->cloudDriveStorage();

        auto parts = Utilities::partPer(usedStorage, totalStorage, PRECISION);
        mUi->pbCloudDrive->setValue(std::min(PRECISION, parts));

        mUi->lUsedCloudDrive->setText(Utilities::getSizeString(usedStorage));

        // ---- Inbox usage
        usedStorage = preferences->inboxStorage();

        mUi->pbInbox->setValue(std::min(PRECISION,
                                       Utilities::partPer(usedStorage, totalStorage, PRECISION)));

        // Display only if not empty. Resize dialog to adequate height.
        if (usedStorage > 0)
        {
            mUi->lUsedInbox->setText(Utilities::getSizeString(usedStorage));
            mUi->wInbox->setVisible(true);
        }
        else
        {
            mUi->wInbox->setVisible(false);
        }

        // ---- Rubbish bin usage
        usedStorage = preferences->rubbishStorage();

        mUi->pbRubbish->setValue(std::min(PRECISION,
                                         Utilities::partPer(usedStorage, totalStorage, PRECISION)));

        mUi->lUsedRubbish->setText(Utilities::getSizeString(usedStorage));

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
    qobject_cast<MegaApplication*>(qApp)->dettachStorageObserver(*this);

    refresh(Preferences::instance());
}

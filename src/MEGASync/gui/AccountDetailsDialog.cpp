#include "AccountDetailsDialog.h"
#include "ui_AccountDetailsDialog.h"

#include "Utilities.h"
#include "MegaApplication.h"
#include "TransferQuota.h"
#include "AccountDetailsManager.h"

#include <QStyle>

using namespace mega;

// Precision to use for progressbars (100 for %, 1000 for ppm...)
static constexpr int PRECISION{100};
static constexpr int DEFAULT_MIN_PERCENTAGE{1};

static constexpr int FONT_SIZE_PX{13};

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

    // Set transfer quota progress bar color to blue
    mUi->wCircularTransfer->setProgressBarGradient(QColor(96, 209, 254), QColor(88, 185, 243));

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/account_details/versions.png"));
    mUi->lVersionIcon->setPixmap(icon.pixmap(24, 24));
    // Get fresh data
    refresh();

    // Disable available storage for business accounts
    if (MegaSyncApp->getMegaApi()->isBusinessAccount())
    {
        mUi->wAvailable->hide();
    }

    // Subscribe to data updates (but detach after 1 callback)
    MegaSyncApp->accountDetailsManager()->attachStorageObserver(*this);
    MegaSyncApp->accountDetailsManager()->updateUserStats(AccountDetailsManager::Flag::ALL,
                                                          true,
                                                          USERSTATS_STORAGECLICKED);
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    MegaSyncApp->accountDetailsManager()->dettachStorageObserver(*this);
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

        QString quotaStringFormat = QString::fromLatin1("<span style='color:%1; font-size:%2px;'>%3</span>");
        // Get font to adapt size to widget if needed
        // Getting font from lUsedStorage considering both
        // lUsedStorage and lUsedTransfer use the same font.
        mUi->lUsedStorage->style()->polish(mUi->lUsedStorage);
        mUi->lUsedTransfer->style()->polish(mUi->lUsedTransfer);
        QFont font (mUi->lUsedStorage->font());
        font.setPixelSize(FONT_SIZE_PX);

        // ---------- Process storage usage

        // Get useful data
        auto totalStorage(preferences->totalStorage());
        auto usedStorage(preferences->usedStorage());

        QString usageColorS;
        QString usedStorageString = Utilities::getSizeString(usedStorage);
        QString totalStorageString;
        QString storageUsageStringFormatted (usedStorageString);

        int parts = 0;

        if (Utilities::isBusinessAccount())
        {
            // Disable over quota and warning
            setProperty("storageState", QLatin1String("ok"));
            mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
            usageColorS = QString::fromLatin1("#333333");
        }
        else
        {
            // Check storage state and set property accordingly
            switch (preferences->getStorageState())
            {
                case MegaApi::STORAGE_STATE_PAYWALL:
                // Fallthrough
                case MegaApi::STORAGE_STATE_RED:
                {
                    mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OVER);
                    setProperty("storageState", QLatin1String("full"));
                    usageColorS = QString::fromLatin1("#D90007");
                    break;
                }
                case MegaApi::STORAGE_STATE_ORANGE:
                {
                    mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_WARNING);
                    setProperty("storageState", QLatin1String("warning"));
                    usageColorS = QString::fromLatin1("#F98400");
                    break;
                }
                case MegaApi::STORAGE_STATE_UNKNOWN:
                // Fallthrough
                case MegaApi::STORAGE_STATE_GREEN:
                // Fallthrough
                default:
                {
                    mUi->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
                    setProperty("storageState", QLatin1String("ok"));
                    usageColorS = QString::fromLatin1("#333333");
                    break;
                }
            }

            parts = usedStorage ?
                        std::max(Utilities::partPer(usedStorage, totalStorage),
                                 DEFAULT_MIN_PERCENTAGE)
                                : 0;

            totalStorageString = Utilities::getSizeString(totalStorage);

            storageUsageStringFormatted = Utilities::getTranslatedSeparatorTemplate().arg(
                usedStorageString,
                totalStorageString);;
        }

        mUi->wCircularStorage->setValue(parts);

        long long availableStorage = totalStorage - usedStorage;
        mUi->lAvailableStorage->setText(Utilities::getSizeString(std::max(0ll, availableStorage)));

        // ---------- Process transfer usage

        // Get useful data
        auto totalTransfer(preferences->totalBandwidth());
        auto usedTransfer(preferences->usedBandwidth());
        auto transferQuotaState(MegaSyncApp->getTransferQuotaState());

        QString usageColorT;
        QString usedTransferString (Utilities::getSizeString(usedTransfer));
        QString totalTransferString;
        QString transferUsageStringFormatted (usedTransferString);

        // Set UI according to state
        switch (transferQuotaState) {
            case QuotaState::OK:
            {
                setProperty("transferState", QLatin1String("ok"));
                mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OK);
                usageColorT = QString::fromLatin1("#333333");
                break;
            }
            case QuotaState::WARNING:
            {
                setProperty("transferState", QLatin1String("warning"));
                mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_WARNING);
                usageColorT = QString::fromLatin1("#F98400");
                break;
            }
            case QuotaState::OVERQUOTA:
            // Fallthrough
            case QuotaState::FULL:
            {
                setProperty("transferState", QLatin1String("full"));
                mUi->wCircularTransfer->setState(CircularUsageProgressBar::STATE_OVER);
                usageColorT = QString::fromLatin1("#D90007");
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
                break;
            }
            case Preferences::ACCOUNT_TYPE_FREE:
            {
                setProperty("accountType", QLatin1String("free"));
                mUi->wCircularTransfer->setTotalValueUnknown(transferQuotaState != QuotaState::FULL
                                                        && transferQuotaState != QuotaState::OVERQUOTA);
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

                auto partsT = usedTransfer ?
                                  std::max(Utilities::partPer(usedTransfer, totalTransfer),
                                           DEFAULT_MIN_PERCENTAGE)
                                           : 0;

                mUi->wCircularTransfer->setValue(partsT);

                totalTransferString = Utilities::getSizeString(totalTransfer);

                transferUsageStringFormatted = Utilities::getTranslatedSeparatorTemplate().arg(
                    usedTransferString,
                    totalTransferString);
                break;
            }
        }

        // Now compute the font size and set usage strings
        // Find correct font size so that the string does not overflow
        auto defaultColor = QString::fromLatin1("#8d8d8e");

        auto contentsMargins = mUi->lUsedStorage->contentsMargins();
        auto margin = contentsMargins.left() + contentsMargins.right() + 2 * mUi->lUsedStorage->margin();
        auto storageStringMaxWidth = mUi->wStorageDetails->contentsRect().width() - margin;

        contentsMargins = mUi->lUsedTransfer->contentsMargins();
        margin = contentsMargins.left() + contentsMargins.right() + 2 * mUi->lUsedTransfer->margin();
        auto transferStringMaxWidth = mUi->wTransferDetails->contentsRect().width() - margin;

        QFontMetrics fMetrics (font);
        while ((fMetrics.horizontalAdvance(storageUsageStringFormatted) >= storageStringMaxWidth
                || fMetrics.horizontalAdvance(transferUsageStringFormatted) >= transferStringMaxWidth)
               && font.pixelSize() > 1)
        {
            font.setPixelSize(font.pixelSize() - 1);
            fMetrics = QFontMetrics(font);
        };

        // Now apply format (color, font size) to Storage usage string
        auto usedStorageStringFormatted = quotaStringFormat.arg(usageColorS,
                                                                QString::number(font.pixelSize()),
                                                                usedStorageString);
        if (totalStorage == 0ULL || Utilities::isBusinessAccount())
        {
            storageUsageStringFormatted = usedStorageStringFormatted;
        }
        else
        {
            storageUsageStringFormatted = Utilities::getTranslatedSeparatorTemplate().arg(
                usedStorageStringFormatted,
                totalStorageString);
            storageUsageStringFormatted = quotaStringFormat.arg(defaultColor,
                                                                QString::number(font.pixelSize()),
                                                                storageUsageStringFormatted);
        }

        mUi->lUsedStorage->setText(storageUsageStringFormatted);

        // Now apply format (color, font size) to Transfer usage string
        auto usedTransferStringFormatted = quotaStringFormat.arg(usageColorT,
                                                                 QString::number(font.pixelSize()),
                                                                 usedTransferString);
        if (totalTransfer == 0ULL || Utilities::isBusinessAccount())
        {
            transferUsageStringFormatted = usedTransferStringFormatted;
        }
        else
        {
            transferUsageStringFormatted = Utilities::getTranslatedSeparatorTemplate().arg(
                usedTransferStringFormatted,
                totalTransferString);
            transferUsageStringFormatted = quotaStringFormat.arg(defaultColor,
                                                                 QString::number(font.pixelSize()),
                                                                 transferUsageStringFormatted);
        }

        mUi->lUsedTransfer->setText(transferUsageStringFormatted);

        // ---------- Process detailed storage usage

        // ---- Cloud drive storage
        auto usedCloudDriveStorage = preferences->cloudDriveStorage();
        parts = usedCloudDriveStorage ?
                    std::max(Utilities::partPer(usedCloudDriveStorage, totalStorage, PRECISION),
                             DEFAULT_MIN_PERCENTAGE)
                                      : 0;
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
    MegaSyncApp->accountDetailsManager()->dettachStorageObserver(*this);

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

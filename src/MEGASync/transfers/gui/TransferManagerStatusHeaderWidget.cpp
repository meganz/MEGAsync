#include "TransferManagerStatusHeaderWidget.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "ServiceUrls.h"
#include "TextDecorator.h"
#include "ui_TransferManagerStatusHeaderWidget.h"

#include <QStyle>

namespace
{
constexpr int TRANSFER_QUOTA_CHECK_TIME_MS(1000);
const QLatin1String TIME_FORMAT("hh:mm:ss");
}

TransferManagerStatusHeaderWidget::TransferManagerStatusHeaderWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::TransferManagerStatusHeaderWidget),
    mStorageQuotaState(mega::MegaApi::STORAGE_STATE_UNKNOWN)
{
    mUi->setupUi(this);

    mUi->wAllPausedBanner->hide();
    mUi->wStorageBanner->hide();
    mUi->wTransferBanner->hide();

    mUi->wAllPausedBanner->setType(BannerWidget::Type::BANNER_INFO);
    mUi->wTransferBanner->setType(BannerWidget::Type::BANNER_ERROR);
    mUi->wStorageBanner->setAutoManageTextUrl(false);

    connect(mUi->wStorageBanner,
            &BannerWidget::linkActivated,
            this,
            &TransferManagerStatusHeaderWidget::onUpgradeClicked,
            Qt::UniqueConnection);

    mTransferQuotaTimer.setInterval(TRANSFER_QUOTA_CHECK_TIME_MS);
    connect(&mTransferQuotaTimer,
            &QTimer::timeout,
            this,
            &TransferManagerStatusHeaderWidget::setTransferQuotaBannerText);

    setAllPausedBannerText();
    setTransferQuotaBannerText();
    setAlmostFullStorageBannerText();
    setFullStorageBannerText();
}

TransferManagerStatusHeaderWidget::~TransferManagerStatusHeaderWidget()
{
    delete mUi;
}

void TransferManagerStatusHeaderWidget::showAllPausedBanner()
{
    mUi->wAllPausedBanner->show();
}

void TransferManagerStatusHeaderWidget::hideAllPausedBanner()
{
    mUi->wAllPausedBanner->hide();
}

void TransferManagerStatusHeaderWidget::showAlmostFullStorageBanner()
{
    mUi->wStorageBanner->setType(BannerWidget::Type::BANNER_WARNING);
    mUi->wStorageBanner->show();
}

void TransferManagerStatusHeaderWidget::showFullStorageBanner()
{
    mUi->wStorageBanner->setType(BannerWidget::Type::BANNER_ERROR);
    mUi->wStorageBanner->show();
}

void TransferManagerStatusHeaderWidget::setAlmostFullStorageBannerText()
{
    mUi->wStorageBanner->setLinkText(tr("Upgrade now"));
    mUi->wStorageBanner->setTitle(tr("Storage almost full"));
    mUi->wStorageBanner->setDescription(tr("Before your storage becomes "
                                           "full and your uploads, syncs and backups stop."));
}

void TransferManagerStatusHeaderWidget::setFullStorageBannerText()
{
    mUi->wStorageBanner->setLinkText(tr("Upgrade now"));
    mUi->wStorageBanner->setTitle(tr("Storage full"));
    mUi->wStorageBanner->setDescription(
        tr("Uploads are disabled and sync and backups are paused."));
}

void TransferManagerStatusHeaderWidget::setTransferQuotaBannerText()
{
    QString bannerText;

    if (Preferences::instance()->accountType() == Preferences::ACCOUNT_TYPE_FREE)
    {
        bannerText =
            tr("[B]Transfer quota exceeded[/B][BR]You can’t continue downloading as you don’t have "
               "enough transfer quota left for this "
               "IP address. To get more quota, upgrade to a Pro account or wait for %1 until more "
               "free "
               "quota becomes available on your IP address. [A]Learn more[/A] about transfer "
               "quota.")
                .arg(MegaSyncApp->getTransferQuota()->getRemainingTransferQuotaTime().toString(
                    TIME_FORMAT));
    }
    else
    {
        bannerText = tr("Transfer quota exceeded.");
    }

    const auto link = ServiceUrls::getTransferQuotaHelpUrl().toString();
    Text::RichText(link).process(bannerText);
    mUi->wTransferBanner->setTitle(bannerText);
}

void TransferManagerStatusHeaderWidget::setAllPausedBannerText()
{
    mUi->wAllPausedBanner->setTitle(tr("All transfers paused."));
}

void TransferManagerStatusHeaderWidget::showTransferBanner(bool show)
{
    if (show)
    {
        if (Preferences::instance()->accountType() == Preferences::ACCOUNT_TYPE_FREE)
        {
            if (!mTransferQuotaTimer.isActive())
            {
                mTransferQuotaTimer.start();
            }
        }
        else
        {
            if (mTransferQuotaTimer.isActive())
            {
                mTransferQuotaTimer.stop();
            }
        }
        mUi->wTransferBanner->show();
    }
    else
    {
        mUi->wTransferBanner->hide();
        mTransferQuotaTimer.stop();
    }
}

void TransferManagerStatusHeaderWidget::updateStorageBannerText()
{
    switch (mStorageQuotaState)
    {
        case mega::MegaApi::STORAGE_STATE_ORANGE:
        {
            setAlmostFullStorageBannerText();
            showAlmostFullStorageBanner();
            break;
        }
        case mega::MegaApi::STORAGE_STATE_RED:
        {
            setFullStorageBannerText();
            showFullStorageBanner();
            break;
        }
        case mega::MegaApi::STORAGE_STATE_GREEN:
        // Fallthrough
        case mega::MegaApi::STORAGE_STATE_PAYWALL:
        // Fallthrough
        case mega::MegaApi::STORAGE_STATE_UNKNOWN:
        // Fallthrough
        default:
        {
            mUi->wStorageBanner->hide();
            break;
        }
    }
}

void TransferManagerStatusHeaderWidget::onUpgradeClicked()
{
    Utilities::upgradeClicked();
}

void TransferManagerStatusHeaderWidget::setStorageQuotaState(int newStorageQuotaState)
{
    mStorageQuotaState = newStorageQuotaState;
}

bool TransferManagerStatusHeaderWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);

        setFullStorageBannerText();
        setAlmostFullStorageBannerText();
        setTransferQuotaBannerText();
        setAllPausedBannerText();
    }
    return QWidget::event(event);
}

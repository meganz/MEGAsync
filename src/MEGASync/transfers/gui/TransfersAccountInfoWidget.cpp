#include "TransfersAccountInfoWidget.h"

#include "AccountDetailsManager.h"
#include "Preferences.h"
#include "ui_TransfersAccountInfoWidget.h"

#include <QStyle>

namespace
{
constexpr int FULL_PERCENTAGE{100};
constexpr const char* STATE_PROPERTY_NAME{"state"};
const QLatin1String STATE_DEFAULT{"default"};
const QLatin1String STATE_ALMOST{"almost"};
const QLatin1String STATE_ALMOST_FULL{"almostfull"};
const QLatin1String STATE_FULL{"full"};
}

TransfersAccountInfoWidget::TransfersAccountInfoWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::TransfersAccountInfoWidget)
{
    mUi->setupUi(this);

    updateStorageText();
    updateStorageBar();
    AccountDetailsManager::instance()->attachStorageObserver(*this);
    AccountDetailsManager::instance()->attachAccountObserver(*this);
}

TransfersAccountInfoWidget::~TransfersAccountInfoWidget()
{
    AccountDetailsManager::instance()->dettachStorageObserver(*this);
    AccountDetailsManager::instance()->dettachAccountObserver(*this);
    delete mUi;
}

void TransfersAccountInfoWidget::updateStorageElements()
{
    updateStorageText();
    updateStorageBar();
}

void TransfersAccountInfoWidget::updateAccountElements()
{
    checkUpgradeButtonVisibility();
}

void TransfersAccountInfoWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateStorageElements();
    }
    QWidget::changeEvent(event);
}

void TransfersAccountInfoWidget::updateStorageText()
{
    auto totalStorage = Preferences::instance()->totalStorage();
    auto usedStorage = Preferences::instance()->usedStorage();

    QString storageText;
    if (totalStorage > 0)
    {
        if (Utilities::isBusinessAccount())
        {
            storageText = Utilities::createSimpleUsedString(usedStorage);
        }
        else
        {
            int percentage = Utilities::partPer(usedStorage, totalStorage);
            mUi->pbStorage->setValue(std::min(percentage, mUi->pbStorage->maximum()));
            storageText = Utilities::createSimpleUsedOfString(usedStorage, totalStorage);
            updateProgressBarStateUntilFull(percentage);
        }
    }

    mUi->lStorage->setText(storageText);
}

void TransfersAccountInfoWidget::updateStorageBar()
{
    switch (Preferences::instance()->getStorageState())
    {
        case mega::MegaApi::STORAGE_STATE_PAYWALL:
        // Fallthrough
        case mega::MegaApi::STORAGE_STATE_RED:
        {
            mUi->pbStorage->setProperty(STATE_PROPERTY_NAME, STATE_FULL);
            break;
        }
        case mega::MegaApi::STORAGE_STATE_ORANGE:
        {
            if (mUi->pbStorage->property(STATE_PROPERTY_NAME) != STATE_ALMOST_FULL)
            {
                mUi->pbStorage->setProperty(STATE_PROPERTY_NAME, STATE_ALMOST);
            }
            break;
        }
        case mega::MegaApi::STORAGE_STATE_UNKNOWN:
        // Fallthrough
        case mega::MegaApi::STORAGE_STATE_GREEN:
        // Fallthrough
        default:
        {
            mUi->pbStorage->setProperty(STATE_PROPERTY_NAME, STATE_DEFAULT);
            break;
        }
    }

    refreshProgressBar();
}

void TransfersAccountInfoWidget::updateProgressBarStateUntilFull(int percentage)
{
    if (percentage >= FULL_PERCENTAGE &&
        mUi->pbStorage->property(STATE_PROPERTY_NAME) != STATE_ALMOST_FULL &&
        Preferences::instance()->getStorageState() != mega::MegaApi::STORAGE_STATE_RED &&
        Preferences::instance()->getStorageState() != mega::MegaApi::STORAGE_STATE_PAYWALL)
    {
        // Force change of style (round border until OQ state is received).
        mUi->pbStorage->setProperty(STATE_PROPERTY_NAME, STATE_ALMOST_FULL);
        refreshProgressBar();
    }
}

void TransfersAccountInfoWidget::refreshProgressBar()
{
    // Forces the update of the style because in some cases
    // the status of the bar is not updated by itself.
    mUi->pbStorage->style()->unpolish(mUi->pbStorage);
    mUi->pbStorage->style()->polish(mUi->pbStorage);
    mUi->pbStorage->update();
}

void TransfersAccountInfoWidget::checkUpgradeButtonVisibility()
{
    mUi->bUpgrade->setVisible(!Utilities::isBusinessAccount());
}

void TransfersAccountInfoWidget::on_bUpgrade_clicked()
{
    Utilities::upgradeClicked();
}

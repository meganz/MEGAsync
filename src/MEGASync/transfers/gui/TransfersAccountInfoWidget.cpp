#include "TransfersAccountInfoWidget.h"

#include "AccountDetailsManager.h"
#include "Preferences.h"
#include "ui_TransfersAccountInfoWidget.h"

TransfersAccountInfoWidget::TransfersAccountInfoWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::TransfersAccountInfoWidget)
{
    mUi->setupUi(this);

    updateAccountElements();
    AccountDetailsManager::instance()->attachAccountObserver(*this);
}

TransfersAccountInfoWidget::~TransfersAccountInfoWidget()
{
    AccountDetailsManager::instance()->dettachAccountObserver(*this);
    delete mUi;
}

void TransfersAccountInfoWidget::updateAccountElements()
{
    auto totalStorage = Preferences::instance()->totalStorage();
    auto usedStorage = Preferences::instance()->usedStorage();

    QString storageText;
    if (totalStorage == 0)
    {
        storageText = QCoreApplication::translate("SettingsDialog", "Data temporarily unavailable");
    }
    else
    {
        if (Utilities::isBusinessAccount())
        {
            storageText = Utilities::createSimpleUsedString(usedStorage);
        }
        else
        {
            int percentage = Utilities::partPer(usedStorage, totalStorage);
            mUi->pbStorage->setValue(std::min(percentage, mUi->pbStorage->maximum()));
            storageText =
                Utilities::createCompleteUsedString(usedStorage, totalStorage, percentage);
        }
    }

    mUi->lStorage->setText(storageText);
}

void TransfersAccountInfoWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateAccountElements();
    }
    QWidget::changeEvent(event);
}

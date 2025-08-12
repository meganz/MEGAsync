#include "AccountTypeWidget.h"

#include "AccountDetailsManager.h"
#include "Preferences.h"
#include "ui_AccountTypeWidget.h"

namespace
{
constexpr const char* ACCOUNT_LEVEL_PROPERTY_NAME{"level"};
const QLatin1String ACCOUNT_LEVEL_FREE{"free"};
const QLatin1String ACCOUNT_LEVEL_PRO{"pro"};
}

AccountTypeWidget::AccountTypeWidget(QWidget* parent):
    QWidget(parent),
    mUi(new Ui::AccountTypeWidget)
{
    mUi->setupUi(this);

    updateAccountText();
    AccountDetailsManager::instance()->attachAccountObserver(*this);
}

AccountTypeWidget::~AccountTypeWidget()
{
    AccountDetailsManager::instance()->dettachAccountObserver(*this);
    delete mUi;
}

void AccountTypeWidget::updateAccountElements()
{
    updateAccountText();
}

bool AccountTypeWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        updateAccountText();
    }
    return QWidget::event(event);
}

void AccountTypeWidget::updateAccountText()
{
    auto level = Preferences::instance()->accountType();
    mUi->bAccountType->setText(Utilities::getReadablePlanFromId(level));
    if (level == Preferences::ACCOUNT_TYPE_FREE)
    {
        mUi->bAccountType->setProperty(ACCOUNT_LEVEL_PROPERTY_NAME, ACCOUNT_LEVEL_FREE);
    }
    else
    {
        mUi->bAccountType->setProperty(ACCOUNT_LEVEL_PROPERTY_NAME, ACCOUNT_LEVEL_PRO);
    }
}

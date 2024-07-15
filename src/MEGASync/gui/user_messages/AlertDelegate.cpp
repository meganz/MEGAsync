#include "AlertDelegate.h"

#include "AlertItem.h"
#include "UserAlert.h"

namespace
{
constexpr int MaxCost = 16;
}

using namespace mega;

AlertDelegate::AlertDelegate()
    : mItems(MaxCost)
{
}

QWidget* AlertDelegate::getWidget(UserAlert* alert, QWidget* parent)
{
    if(!alert)
    {
        return nullptr;
    }

    auto alertItem = mItems[alert->getId()];
    if(alertItem)
    {
        return alertItem;
    }
    else
    {
        AlertItem* item = new AlertItem(parent);
        item->setAlertData(alert);
        item->show();
        item->hide();
        mItems.insert(alert->getId(), item);
        return mItems[alert->getId()];
    }
}

#include "UserMessageProxyModel.h"

#include "UserAlert.h"

#include "megaapi.h"

#include <QDateTime>

UserMessageProxyModel::UserMessageProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , mActualFilter(MessageType::ALL)
{
}

MessageType UserMessageProxyModel::getActualFilter()
{
    return mActualFilter;
}

void UserMessageProxyModel::setFilter(MessageType filter)
{
    mActualFilter = filter;
    invalidateFilter();
}

bool UserMessageProxyModel::filterAcceptsRow(int row, const QModelIndex& sourceParent) const
{
    if(mActualFilter == MessageType::ALL)
    {
        return true;
    }

    bool filter = false;
    QModelIndex index = sourceModel()->index(row, 0, sourceParent);
    UserMessage* item = static_cast<UserMessage*>(index.internalPointer());
    if(item)
    {
        filter = item->isRowAccepted(mActualFilter);
    }
    return filter;
}

bool UserMessageProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    UserMessage* leftItem = static_cast<UserMessage*>(left.internalPointer());
    UserMessage* rightItem = static_cast<UserMessage*>(right.internalPointer());

    if(!leftItem || !rightItem)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid items (lessThan).");
        return false;
    }

    bool isLess(false);

    if(leftItem->getType() != rightItem->getType())
    {
        isLess = leftItem->getType() < rightItem->getType();
    }
    else
    {
        isLess = leftItem->sort(rightItem);
    }

    return isLess;
}

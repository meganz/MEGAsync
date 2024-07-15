#ifndef USER_MESSAGE_PROXY_MODEL_H
#define USER_MESSAGE_PROXY_MODEL_H

#include "NotificationAlertTypes.h"

#include <QSortFilterProxyModel>

class UserMessageProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    UserMessageProxyModel(QObject* parent = 0);
    virtual ~UserMessageProxyModel() = default;

    UserMessageType getActualFilter();
    void setFilter(UserMessageType filter);

protected:
    bool filterAcceptsRow(int row, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    UserMessageType mActualFilter;

};

#endif // USER_MESSAGE_PROXY_MODEL_H

#ifndef USER_MESSAGE_PROXY_MODEL_H
#define USER_MESSAGE_PROXY_MODEL_H

#include "UserMessageTypes.h"

#include <QSortFilterProxyModel>

class UserMessageProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    UserMessageProxyModel(QObject* parent = 0);
    virtual ~UserMessageProxyModel() = default;

    MessageType getActualFilter();
    void setFilter(MessageType filter);

protected:
    bool filterAcceptsRow(int row, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    MessageType mActualFilter;

};

#endif // USER_MESSAGE_PROXY_MODEL_H

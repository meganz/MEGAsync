#ifndef MEGANOTIFICATIONDELEGATE_H
#define MEGANOTIFICATIONDELEGATE_H

#include "NotificationModel.h"

#include <QStyledItemDelegate>

class MegaNotificationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaNotificationDelegate(NotificationModel* notificationModel, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    NotificationModel* mNotificationModel;

};

#endif // MEGANOTIFICATIONDELEGATE_H

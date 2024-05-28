#ifndef MEGANOTIFICATIONDELEGATE_H
#define MEGANOTIFICATIONDELEGATE_H

#include "QNotificationsModel.h"

#include <QStyledItemDelegate>

class MegaNotificationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaNotificationDelegate(QNotificationsModel* notificationModel, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    QNotificationsModel* mNotificationModel;

};

#endif // MEGANOTIFICATIONDELEGATE_H

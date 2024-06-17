#ifndef NOTIFICATIONALERTDELEGATE_H
#define NOTIFICATIONALERTDELEGATE_H

#include "MegaNotificationDelegate.h"
#include "AlertDelegate.h"

class NotificationAlertDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NotificationAlertDelegate(MegaNotificationDelegate* notificationsDelegate,
                              AlertDelegate* alertsDelegate,
                              QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    MegaNotificationDelegate* mNotificationsDelegate;
    AlertDelegate* mAlertsDelegate;

};

#endif // NOTIFICATIONALERTDELEGATE_H

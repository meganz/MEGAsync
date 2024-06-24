#ifndef NOTIFICATIONALERTDELEGATE_H
#define NOTIFICATIONALERTDELEGATE_H

#include "NotificationDelegate.h"
#include "AlertDelegate.h"

class NotificationAlertDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NotificationAlertDelegate(NotificationDelegate* notificationsDelegate,
                              AlertDelegate* alertsDelegate,
                              QObject *parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    std::unique_ptr<NotificationDelegate> mNotificationsDelegate;
    std::unique_ptr<AlertDelegate> mAlertsDelegate;

};

#endif // NOTIFICATIONALERTDELEGATE_H

#ifndef NOTIFICATIONALERTDELEGATE_H
#define NOTIFICATIONALERTDELEGATE_H

#include "NotificationDelegate.h"
#include "AlertDelegate.h"

class NotificationAlertDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;
    virtual ~NotificationAlertDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    void createNotificationDelegate(NotificationModel* model);
    void createAlertDelegate(AlertModel* model);

private:
    std::unique_ptr<NotificationDelegate> mNotificationsDelegate = nullptr;
    std::unique_ptr<AlertDelegate> mAlertsDelegate = nullptr;

};

#endif // NOTIFICATIONALERTDELEGATE_H

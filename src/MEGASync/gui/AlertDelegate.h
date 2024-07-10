#ifndef ALERT_DELEGATE_H
#define ALERT_DELEGATE_H

#include "AlertModel.h"
#include "NotificationAlertTypes.h"

#include <QStyledItemDelegate>

class AlertDelegate
{
public:
    AlertDelegate();
    /*
    void paint(QPainter* painter, const QStyleOptionViewItem& option,const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
    bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index);
    */
    QWidget* getWidget(MegaUserAlertExt* alert);

private:
    QCache<int, AlertItem> mItems;

    //void handleAlertItem(MegaUserAlertExt* alert, const QRect& rect, QPainter* painter) const;
    //NotificationAlertModelItem* getModelItem(const QModelIndex& index) const;

};

#endif // ALERT_DELEGATE_H

#ifndef ALERTDELEGATE_H
#define ALERTDELEGATE_H

#include <QStyledItemDelegate>
#include "AlertModel.h"
#include "AlertItem.h"

class AlertDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    AlertDelegate(AlertModel *model, bool useProxyModel, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    AlertModel* mAlertsModel;
    bool mUseProxy;
};

#endif // ALERTDELEGATE_H

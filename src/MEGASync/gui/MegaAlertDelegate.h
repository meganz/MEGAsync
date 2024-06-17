#ifndef MEGAALERTDELEGATE_H
#define MEGAALERTDELEGATE_H

#include <QStyledItemDelegate>
#include "AlertModel.h"
#include "AlertItem.h"

class MegaAlertDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaAlertDelegate(AlertModel *model, bool useProxyModel, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    AlertModel* mAlertsModel;
    bool mUseProxy;
};

#endif // MEGAALERTDELEGATE_H

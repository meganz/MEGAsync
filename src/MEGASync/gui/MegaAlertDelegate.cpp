#include "MegaAlertDelegate.h"
#include <QPainter>
#include "megaapi.h"
#include <QDebug>
#include "QSortFilterProxyModel.h"

using namespace mega;

MegaAlertDelegate::MegaAlertDelegate(QAlertsModel *model, bool useProxyModel, QObject *parent)
    : QStyledItemDelegate(parent)
{
    this->model = model;
    useProxy = useProxyModel;
}

void MegaAlertDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {       

        //Map index when we are using QSortFilterProxyModel
        // if we are using QAbstractItemModel just access internalPointer casting to MegaAlert
        MegaUserAlert *alert = NULL;
        if (useProxy)
        {
            alert = (MegaUserAlert *)((((QSortFilterProxyModel*)index.model())->mapToSource(index)).internalPointer());
        }
        else
        {
            alert = (MegaUserAlert *)index.internalPointer();
        }

        AlertItem *ti = model->alertItems[alert->getId()];
        if (!ti)
        {
            ti = new AlertItem();
            ti->setAlertData(alert);
            model->alertItems.insert(alert->getId(), ti);
        }
        else
        {
            ti->setAlertData(alert);
        }

        painter->save();
        painter->translate(option.rect.topLeft());
        ti->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));
        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaAlertDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {
        return QSize(400, 122);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

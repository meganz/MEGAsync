#include "StalledIssueDelegate.h"

#include <QPainter>

StalledIssueDelegate::StalledIssueDelegate(StalledIssuesProxyModel* proxyModel,  QAbstractItemView* view)
    :mView(view),
     mProxyModel (proxyModel),
     mSourceModel (qobject_cast<StalledIssuesModel*>(
                       mProxyModel->sourceModel())),
     QStyledItemDelegate(view)
{
    mStalledIssueHeaderItems.resize(10);
    mStalledIssueInfoItems.resize(10);
}

QSize StalledIssueDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const
{
    StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index));
    if(w)
    {
       return  w->sizeHint();
    }

    return QSize();
}

void StalledIssueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto rowCount (index.model()->rowCount());
    auto row (index.row());

    if (index.isValid() && row < rowCount)
    {
        auto pos (option.rect.topLeft());
        auto width (option.rect.width());
        auto height (option.rect.height());
        auto stalledIssueItem (qvariant_cast<StalledIssue>(index.data(Qt::DisplayRole)));
        auto data = stalledIssueItem.getStalledIssueData();

        StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index));
        if(!w)
        {
            return;
        }

        // Move if position changed
        if (w->pos() != pos)
        {
            w->move(pos);
        }

        // Resize if window resized
        if (w->width() != width)
        {
            w->resize(width, height);
        }

        w->updateUi(data, row);
        painter->save();
        painter->translate(pos);
        w->render(option, painter, QRegion(0, 0, width, height));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

//bool StalledIssueDelegate::event(QEvent *event)
//{

//}

//bool StalledIssueDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
//{

//}

//bool StalledIssueDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
//{

//}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getStalledIssueItemWidget(const QModelIndex &index) const
{
    int row(0);
    StalledIssueBaseDelegateWidget* item(nullptr);

    if(index.parent().isValid())
    {
        row = index.parent().row() % 10;

        if(row >= mStalledIssueInfoItems.size())
        {
           item = mProxyModel->createTransferManagerItem(index, mView);
           mStalledIssueInfoItems.append(item);
        }
        else
        {
            item = mStalledIssueInfoItems.at(row);
        }
    }
    else
    {
        row = index.row() % 10;

        if(row >= mStalledIssueHeaderItems.size())
        {
           item = mProxyModel->createTransferManagerItem(index, mView);
           mStalledIssueHeaderItems.append(item);
        }
        else
        {
            item = mStalledIssueHeaderItems.at(row);
        }
    }

    item->setCurrentIndex(index);

    return item;
}

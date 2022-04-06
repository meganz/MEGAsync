#include "StalledIssueDelegate.h"

#include "StalledIssueFilePath.h"
#include "StalledIssueBaseDelegateWidget.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesView.h"
#include "MegaDelegateHoverManager.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

StalledIssueDelegate::StalledIssueDelegate(StalledIssuesProxyModel* proxyModel,  StalledIssuesView *view)
    :mView(view),
     mProxyModel (proxyModel),
     mSourceModel (qobject_cast<StalledIssuesModel*>(
                       mProxyModel->sourceModel())),
     mEditor(nullptr),
     QStyledItemDelegate(view)
{
    mCacheManager.setProxyModel(mProxyModel);
}

StalledIssueDelegate::~StalledIssueDelegate()
{
}

QSize StalledIssueDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex& index) const
{
    auto stalledIssueItem (qvariant_cast<StalledIssue>(index.data(Qt::DisplayRole)));
    auto data = stalledIssueItem.getStalledIssueData();
    StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, data));
    if(w)
    {
       auto size = w->sizeHint();

       return  size;
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

        painter->fillRect(option.rect,Qt::white);


        painter->save();
        painter->translate(pos);

        StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, data));
        if(!w)
        {
            return;
        }


        w->setGeometry(option.rect);
        w->render(option, painter, QRegion(0, 0, width, height));

        bool drawBottomLine(false);

        if(index.parent().isValid())
        {
            drawBottomLine = true;
        }
        else
        {
            if(!mView->isExpanded(index))
            {
                drawBottomLine = true;
            }
        }

        if(drawBottomLine)
        {
            QPen pen = painter->pen();
            pen.setWidth(1);
            pen.setColor(QColor("#000000"));
            painter->setPen(pen);
            painter->setOpacity(0.08);
            painter->drawLine(QPoint(0, option.rect.height() -1),QPoint(option.rect.width(),option.rect.height() -1));
        }

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool StalledIssueDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        if(auto mouseButtonEvent = dynamic_cast<QMouseEvent*>(event))
        {
            if(mouseButtonEvent->button() == Qt::LeftButton)
            {
                if(index.isValid())
                {
                    if(index.parent().isValid())
                    {
                        auto parentIndex = index.parent();
                        qDebug() << index << index.parent() << mView->isExpanded(parentIndex);
                        mView->isExpanded(parentIndex) ? mView->collapse(parentIndex) : mView->expand(parentIndex);
                    }
                    else
                    {

                        qDebug() << index  << mView->isExpanded(index);
                        mView->isExpanded(index) ? mView->collapse(index) : mView->expand(index);
                    }
                }
            }
        }
    }

   return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QWidget *StalledIssueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto stalledIssueItem (qvariant_cast<StalledIssue>(index.data(Qt::DisplayRole)));
    auto data = stalledIssueItem.getStalledIssueData();

    mEditor = getNonCacheStalledIssueItemWidget(index,parent, data);
    mEditor->setStyleSheet(QString::fromUtf8("*{background-color: transparent;}"));
    mEditor->setGeometry(option.rect);

    connect(mEditor, &QObject::destroyed, this, [this](){
        mEditor = nullptr;
    });

    return mEditor;
}

void StalledIssueDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool StalledIssueDelegate::event(QEvent *event)
{
    if(auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        if(hoverEvent->type() == QEvent::Leave)
        {
            onHoverLeave(hoverEvent->index(),hoverEvent->rect());
        }
        else if(hoverEvent->type() == QEvent::Enter)
        {
            onHoverEnter(hoverEvent->index(), hoverEvent->rect());
        }
        else if(hoverEvent->type() == QEvent::MouseMove)
        {
            //onHoverMove(hoverEvent->index(), hoverEvent->rect(), hoverEvent->mousePos());
        }
    }

    return QStyledItemDelegate::event(event);

}

bool StalledIssueDelegate::eventFilter(QObject *object, QEvent *event)
{
    return QStyledItemDelegate::eventFilter(object, event);
}

void StalledIssueDelegate::onHoverLeave(const QModelIndex &index, const QRect &rect)
{
    mView->closeEditor(mEditor, QAbstractItemDelegate::NoHint);
    mEditor = nullptr;
}

void StalledIssueDelegate::onHoverEnter(const QModelIndex &index, const QRect &rect)
{
    if(mEditor)
    {
        mView->closeEditor(mEditor, QAbstractItemDelegate::NoHint);
        mEditor = nullptr;
    }
    mView->setCurrentIndex(index);
    mView->edit(index);
}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getStalledIssueItemWidget(const QModelIndex &index, StalledIssueDataPtr data) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);

    if(index.parent().isValid())
    {
        item = mCacheManager.getStalledIssueInfoWidget(index,mView, data);
    }
    else
    {
        item = mCacheManager.getStalledIssueHeaderWidget(index,mView, data);
    }

    return item;
}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getNonCacheStalledIssueItemWidget(const QModelIndex &index, QWidget* parent, StalledIssueDataPtr data) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);

    if(index.parent().isValid())
    {
        item = mCacheManager.getNonCacheStalledIssueInfoWidget(index,parent, data);
    }
    else
    {
        item = mCacheManager.getNonCacheStalledIssueHeaderWidget(index,parent, data);
    }

    return item;
}

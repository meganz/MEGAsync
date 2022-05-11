#include "StalledIssueDelegate.h"

#include "StalledIssueFilePath.h"
#include "StalledIssueBaseDelegateWidget.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesView.h"
#include "MegaDelegateHoverManager.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QElapsedTimer>

StalledIssueDelegate::StalledIssueDelegate(StalledIssuesProxyModel* proxyModel,  StalledIssuesView *view)
    :mView(view),
     mProxyModel (proxyModel),
     mSourceModel (qobject_cast<StalledIssuesModel*>(
                       mProxyModel->sourceModel())),
     QStyledItemDelegate(view),
     mEditor(nullptr)
{
    mCacheManager.setProxyModel(mProxyModel);
}

StalledIssueDelegate::~StalledIssueDelegate()
{
}

QSize StalledIssueDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if(!index.parent().isValid())
    {
        return QSize(200,StalledIssueHeader::HEIGHT);
    }
    else
    {
        auto stalledIssueItem (qvariant_cast<StalledIssue>(index.data(Qt::DisplayRole)));
        StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, stalledIssueItem));
        if(w)
        {
            return w->sizeHint();
        }
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

void StalledIssueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QElapsedTimer timer;
    timer.start();
    QList<double> times;

    auto rowCount (index.model()->rowCount());
    auto row (index.row());

    if (index.isValid() && row < rowCount)
    {
        auto pos (option.rect.topLeft());

        QColor rowColor;

        if(index.parent().isValid())
        {
            rowColor = Qt::white;
        }
        else
        {
            if(index.row()%2 == 0)
            {
                rowColor = Qt::white;
            }
            else
            {
                rowColor.setRed(0);
                rowColor.setGreen(0);
                rowColor.setBlue(0);
                rowColor.setAlphaF(0.05);
            }
        }


        painter->fillRect(option.rect, rowColor);

        times.append(timer.nsecsElapsed()/1000000.0);

        painter->save();
        painter->translate(pos);

        if(!mEditor || mEditor->getCurrentIndex() != index)
        {
            auto stalledIssueItem (qvariant_cast<StalledIssue>(index.data(Qt::DisplayRole)));
            StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, stalledIssueItem));
            if(!w)
            {
                return;
            }

            times.append(timer.nsecsElapsed()/1000000.0);

            w->expand(mView->isExpanded(index));
            w->setGeometry(option.rect);
            w->render(option, painter, QRegion(0, 0, option.rect.width(), option.rect.height()));
        }


        times.append(timer.nsecsElapsed()/1000000.0);

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
            painter->drawLine(QPoint(0 - option.rect.x(), option.rect.height() -1),QPoint(mView->width(),option.rect.height() -1));
        }


        times.append(timer.nsecsElapsed()/1000000.0);

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
                    if(!index.parent().isValid())
                    {
                        auto currentState = mView->isExpanded(index);
                        currentState ? mView->collapse(index) : mView->expand(index);

                        if(mEditor)
                        {
                            mEditor->expand(!currentState);

                            //If it is going to be expanded
                            if(!currentState)
                            {
                                auto childIndex = index.child(0,0);
                                if(childIndex.isValid())
                                {
                                    mView->scrollTo(childIndex);
                                }
                            }
                        }
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

    mEditor = getNonCacheStalledIssueItemWidget(index,parent, stalledIssueItem);
    mEditor->expand(mView->isExpanded(index));
    mEditor->setGeometry(option.rect);

    return mEditor;
}

void StalledIssueDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

bool StalledIssueDelegate::event(QEvent *event)
{
    if(auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        if(hoverEvent->type() == QEvent::Enter)
        {
            onHoverEnter(hoverEvent->index());
        }
        else if(hoverEvent->type() == QEvent::Leave)
        {
            onHoverLeave(hoverEvent->index());
        }
        else if(hoverEvent->type() == QEvent::MouseMove)
        {
            if(!mEditor)
            {
                onHoverEnter(hoverEvent->index());
            }
        }
    }

    return QStyledItemDelegate::event(event);
}

bool StalledIssueDelegate::eventFilter(QObject *object, QEvent *event)
{
    return QStyledItemDelegate::eventFilter(object, event);
}

void StalledIssueDelegate::onHoverEnter(const QModelIndex &index)
{
    mView->setCurrentIndex(index);
    mView->edit(index);
}

void StalledIssueDelegate::onHoverLeave(const QModelIndex& index)
{
    if(mEditor)
    {
        //Small hack to avoid blinks when changing from editor to delegate paint
        auto editor = mEditor.data();
        mEditor = nullptr;
        mView->update(index);
        mView->closeEditor(editor, QAbstractItemDelegate::NoHint);
    }
}

void StalledIssueDelegate::onIssueFixed()
{
   auto senderWidget = dynamic_cast<StalledIssueBaseDelegateWidget*>(sender());
   if(senderWidget)
   {
       auto index = senderWidget->getCurrentIndex();
       mSourceModel->finishStalledIssues(QModelIndexList() << index);
   }
}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getStalledIssueItemWidget(const QModelIndex &index, const StalledIssue& data) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);

    if(index.parent().isValid())
    {
        item = mCacheManager.getStalledIssueInfoWidget(index,mView->viewport(), data);
    }
    else
    {
        item = mCacheManager.getStalledIssueHeaderWidget(index,mView->viewport(), data);
    }

    return item;
}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getNonCacheStalledIssueItemWidget(const QModelIndex &index, QWidget* parent, const StalledIssue &data) const
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

    connect(item, &StalledIssueBaseDelegateWidget::issueFixed, this, &StalledIssueDelegate::onIssueFixed);

    return item;
}

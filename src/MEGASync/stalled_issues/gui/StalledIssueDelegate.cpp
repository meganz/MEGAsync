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
    :QStyledItemDelegate(view),
     mView(view),
     mProxyModel (proxyModel),
     mEditor(nullptr)
{
    mSourceModel = qobject_cast<StalledIssuesModel*>(
                      mProxyModel->sourceModel());

    mCacheManager.setProxyModel(mProxyModel);
    connect(mSourceModel, &QAbstractItemModel::modelReset, this, [this](){
        resetCache();
    });
}

StalledIssueDelegate::~StalledIssueDelegate()
{
}

QSize StalledIssueDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool adaptativeHeight(true);

    if(!index.parent().isValid())
    {
       adaptativeHeight = index.data(StalledIssuesModel::ADAPTATIVE_HEIGHT_ROLE).toBool();
    }

    if(adaptativeHeight)
    {
        auto stalledIssueItem (qvariant_cast<StalledIssueVariant>(index.data(Qt::DisplayRole)));
        StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, stalledIssueItem));
        if(w)
        {
            return w->sizeHint();
        }
    }
    else
    {
        return QSize(200,StalledIssueHeader::HEIGHT);
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

void StalledIssueDelegate::resetCache()
{
    mCacheManager.reset();
}

void StalledIssueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto rowCount (index.model()->rowCount());
    auto row (index.row());

    if (index.isValid() && row < rowCount)
    {
        bool isExpanded(mView->isExpanded(index));

        auto pos (option.rect.topLeft());

        QColor rowColor = getRowColor(index.parent().isValid() ? index.parent() : index);
        auto backgroundRect = isExpanded ? option.rect : option.rect.adjusted(0,0,0,-1);
        painter->fillRect(backgroundRect, rowColor);

        painter->save();
        painter->translate(pos);

        bool renderDelegate(!mEditor || mEditor->getCurrentIndex() != index);

        if(renderDelegate)
        {
            auto stalledIssueItem (qvariant_cast<StalledIssueVariant>(index.data(Qt::DisplayRole)));
            StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, stalledIssueItem));
            if(!w)
            {
                return;
            }

            painter->save();

            w->expand(isExpanded);
            w->setGeometry(option.rect);

            auto pixmap = w->grab();
            painter->drawPixmap(0,0,w->width(), w->height(),pixmap);

            painter->restore();
        }

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
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setPen(QPen(Qt::black, 1));
            painter->setOpacity(0.2);
            painter->drawLine(QPoint(0 - option.rect.x(), option.rect.height()-1),QPoint(mView->width(),option.rect.height()-1));
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
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QWidget *StalledIssueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto stalledIssueItem (qvariant_cast<StalledIssueVariant>(index.data(Qt::DisplayRole)));

    if(stalledIssueItem.consultData())
    {
        mEditor = getNonCacheStalledIssueItemWidget(index,parent, stalledIssueItem);
        mEditor->expand(mView->isExpanded(index));
        mEditor->setGeometry(option.rect);
    }

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
        else if(hoverEvent->type() == QEvent::MouseMove)
        {
            onHoverEnter(hoverEvent->index());
        }
    }

    return QStyledItemDelegate::event(event);
}

bool StalledIssueDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        if(auto mouseButtonEvent = dynamic_cast<QMouseEvent*>(event))
        {
            if(mouseButtonEvent->button() == Qt::LeftButton)
            {
                auto viewPos = mView->mapFromGlobal(mouseButtonEvent->globalPos());
                auto index = mView->indexAt(viewPos);

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

                            return true;
                        }
                    }
                }
            }
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

void StalledIssueDelegate::onHoverEnter(const QModelIndex &index)
{
    if(mEditor && mEditor->getCurrentIndex() != index)
    {
        //Small hack to avoid blinks when changing from editor to delegate paint
        //Set the editor to nullptr and update the view -> Then the delegate paints the base widget
        //before the editor is removed
        auto editorIndex = mEditor->getCurrentIndex();
        mEditor = nullptr;
        mView->update(index);
        mView->closePersistentEditor(editorIndex);
    }

    if(!mEditor)
    {
        mView->setCurrentIndex(index);
        mView->openPersistentEditor(index);
    }
}

QColor StalledIssueDelegate::getRowColor(const QModelIndex &index) const
{
    QColor rowColor;

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

    return rowColor;
}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getStalledIssueItemWidget(const QModelIndex &index, const StalledIssueVariant& data) const
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

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getNonCacheStalledIssueItemWidget(const QModelIndex &index, QWidget* parent, const StalledIssueVariant &data) const
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

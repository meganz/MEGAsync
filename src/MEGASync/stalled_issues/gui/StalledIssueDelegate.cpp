#include "StalledIssueDelegate.h"

#include "StalledIssueFilePath.h"
#include "StalledIssueBaseDelegateWidget.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesView.h"
#include "MegaDelegateHoverManager.h"
#include "StalledIssue.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QElapsedTimer>

const QColor HOVER_COLOR = QColor("#FAFAFA");
const QColor SELECTED_BORDER_COLOR = QColor("#E9E9E9");
const float LEFT_MARGIN = 4.0;
const float RIGHT_MARGIN = 6.0;
const float RECT_BORDERS_MARGIN = 20.0;
const float BOTTON_MARGIN = 6.0;
const float TOP_MARGIN = 2.0;
const float CORNER_RADIUS = 10.0;
const int PEN_WIDTH = 2;
const int UPDATE_SIZE_TIMER = 50;

StalledIssueDelegate::StalledIssueDelegate(StalledIssuesProxyModel* proxyModel,  StalledIssuesView *view)
    :QStyledItemDelegate(view),
     mView(view),
     mProxyModel (proxyModel),
     mCacheManager(this),
     mEditor(nullptr)
{
    mSourceModel = qobject_cast<StalledIssuesModel*>(
                      mProxyModel->sourceModel());

    mCacheManager.setProxyModel(mProxyModel);

    mUpdateSizeHintTimer.setSingleShot(true);
    connect(&mUpdateSizeHintTimer, &QTimer::timeout, [this](){
        emit sizeHintChanged(QModelIndex());
    });

    connect(mView, &StalledIssuesView::scrollStopped, this, [this](){
        updateVisibleIndexesSizeHint(UPDATE_SIZE_TIMER);
    });

    mUpdateSizeHintTimerFromResize.setSingleShot(true);
    connect(&mUpdateSizeHintTimerFromResize, &QTimer::timeout, this, [this](){
        mVisibleIndexesRange.clear();
        //Update it as soon as possible
        updateVisibleIndexesSizeHint(UPDATE_SIZE_TIMER);
    });

    mView->installEventFilter(this);
}

void StalledIssueDelegate::updateVisibleIndexesSizeHint(int updateDelay)
{
    auto firstIndex = mView->indexAt(QPoint(0,0));
    auto firstRow = firstIndex.parent().isValid() ? firstIndex.parent().row() : firstIndex.row();

    for(int row = firstRow - 15; row <= (firstRow + 15); ++row)
    {
        if(row >= 0)
        {
            if(!mVisibleIndexesRange.contains(row))
            {
                auto index = mProxyModel->index(row, 0);
                if(index.isValid())
                {
                    if(mVisibleIndexesRange.size() == 30)
                    {
                        mVisibleIndexesRange.removeFirst();
                    }

                    mVisibleIndexesRange.append(row);

                    StalledIssueVariant stalledIssueItem (qvariant_cast<StalledIssueVariant>(index.data(Qt::DisplayRole)));
                    stalledIssueItem.removeDelegateSize(StalledIssue::Header);
                }

                mUpdateSizeHintTimer.start(updateDelay);
            }
        }
    }
}

QSize StalledIssueDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    StalledIssueVariant stalledIssueItem (qvariant_cast<StalledIssueVariant>(index.data(Qt::DisplayRole)));
    {
        StalledIssue::SizeType sizeType = index.parent().isValid() ? StalledIssue::Body : StalledIssue::Header;
        auto size = stalledIssueItem.getDelegateSize(sizeType);
        if(size.isValid())
        {
            return size;
        }

        StalledIssueBaseDelegateWidget* w (getStalledIssueItemWidget(index, stalledIssueItem));
        if(w)
        {
            size = w->sizeHint();
            return size;
        }
    }

    return QStyledItemDelegate::sizeHint(option, index);
}

void StalledIssueDelegate::resetCache()
{
    mCacheManager.reset();
    mMouseHoverOrSelectedLastState.clear();
}

void StalledIssueDelegate::updateSizeHint()
{
    mUpdateSizeHintTimer.start(UPDATE_SIZE_TIMER);
}

void StalledIssueDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto rowCount (index.model()->rowCount());
    auto row (index.row());

    if (index.isValid() && row < rowCount)
    {
        bool isExpanded(mView->isExpanded(index));

        auto pos (option.rect.topLeft());

        auto rowColor = Qt::white;
        auto backgroundRect = isExpanded ? option.rect : option.rect.adjusted(0,0,0,-1);

        QStyle::State parentState = option.state;

        auto detectRelativeHover =   [this, index, &parentState](){
            QPoint cursorPos = QCursor::pos();
            auto viewPos = mView->mapFromGlobal(cursorPos);
            if(viewPos.y() >= 0 && mView->isActiveWindow())
            {
                auto hoverIndex = mView->indexAt(viewPos);
                if(hoverIndex.isValid() && (index.parent() == hoverIndex ||
                                            index == hoverIndex.parent() ||
                                            index == hoverIndex))
                {
                    parentState |= QStyle::State_MouseOver;
                }
            }
        };

        if(index.parent().isValid())
        {
            if(mView->selectionModel()->isSelected(index.parent()))
            {
                parentState|= QStyle::State_Selected;
            }
        }

        detectRelativeHover();

        //The selected/hover state has changed
        bool isHoverOrSelected((parentState & QStyle::State_MouseOver) && (parentState & QStyle::State_Selected));
        if(isHoverOrSelected != mMouseHoverOrSelectedLastState.value(index, false))
        {
            mView->update(index.parent().isValid() ? index.parent() : index.model()->index(0,0,index));

            mMouseHoverOrSelectedLastState.insert(index, isHoverOrSelected);
        }

        QColor fillColor;
        QPen pen;
        QPainterPath path;

        if(parentState & (QStyle::State_MouseOver | QStyle::State_Selected))
        {
            path.setFillRule( Qt::WindingFill );
            path.addRoundedRect(QRectF(option.rect.x() + LEFT_MARGIN,
                                       option.rect.y() + TOP_MARGIN,
                                       option.rect.width() - RIGHT_MARGIN,
                                       option.rect.height() - BOTTON_MARGIN), CORNER_RADIUS, CORNER_RADIUS);

            if(index.parent().isValid())
            {
                path.addRect(QRectF(option.rect.x() + LEFT_MARGIN,
                                    option.rect.y(),
                                    option.rect.width() - RIGHT_MARGIN,
                                    RECT_BORDERS_MARGIN)); // Bottom corners not rounded
            }
            else if(isExpanded)
            {
                path.addRect(QRectF(option.rect.x() + LEFT_MARGIN,
                                    option.rect.y() + RECT_BORDERS_MARGIN,
                                    option.rect.width() - RIGHT_MARGIN,
                                    option.rect.height() - RECT_BORDERS_MARGIN)); // Bottom corners not rounded
            }

            if(parentState & QStyle::State_MouseOver && parentState & QStyle::State_Selected)
            {
                pen.setColor(SELECTED_BORDER_COLOR);
                pen.setWidth(PEN_WIDTH);
                fillColor = HOVER_COLOR;
            }
            else
            {
                if(parentState & QStyle::State_MouseOver)
                {
                    pen.setColor(HOVER_COLOR);
                    fillColor = HOVER_COLOR;
                }
                else if (parentState & QStyle::State_Selected)
                {
                    pen.setColor(SELECTED_BORDER_COLOR);
                    pen.setWidth(PEN_WIDTH);
                    fillColor = Qt::white;
                }
            }

            if(pen != QPen())
            {
                painter->setPen(pen);
            }
        }

        painter->fillRect(backgroundRect, rowColor);
        if(!path.isEmpty())
        {
            painter->fillPath(path.simplified(), fillColor);
            painter->drawPath(path.simplified());
        }

        //REMOVE SEPARATION LINE WHEN EXPANDED
        if(parentState & (QStyle::State_MouseOver | QStyle::State_Selected))
        {
            QRectF collision;

            if(index.parent().isValid())
            {
                collision = QRectF(option.rect.x() + LEFT_MARGIN + PEN_WIDTH/2 /* next pixel*/,
                                   option.rect.y() - PEN_WIDTH/2,
                                   option.rect.x() + option.rect.width() - RIGHT_MARGIN - PEN_WIDTH,
                                   PEN_WIDTH);

            }
            else if(isExpanded)
            {
                collision = QRectF(option.rect.x() + LEFT_MARGIN + PEN_WIDTH/2 /* next pixel*/,
                                   option.rect.y() + option.rect.height() - PEN_WIDTH/2,
                                   option.rect.x() + option.rect.width() - RIGHT_MARGIN - PEN_WIDTH,
                                   PEN_WIDTH);
            }

            painter->fillRect(collision, fillColor);
        }

        painter->save();
        painter->translate(pos);

        QModelIndex editorCurrentIndex(getEditorCurrentIndex());

        bool renderDelegate(editorCurrentIndex != index);

        QRect geometry(option.rect);

#ifdef __APPLE__
        auto width = mView->width();
        width -= mView->contentsMargins().left();
        width -= mView->contentsMargins().right();
        if(mView->verticalScrollBar() && mView->verticalScrollBar()->isVisible())
        {
            width -= mView->verticalScrollBar()->width();
        }
        geometry.setWidth(width);
#endif

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
            w->setGeometry(geometry);

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
            painter->drawLine(QPoint(0 - geometry.x(), geometry.height()-1),QPoint(mView->width(), geometry.height()-1));
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

QWidget *StalledIssueDelegate::createEditor(QWidget*, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto stalledIssueItem (qvariant_cast<StalledIssueVariant>(index.data(Qt::DisplayRole)));

    if(stalledIssueItem.consultData())
    {
        mEditor = getStalledIssueItemWidget(index, stalledIssueItem);
        mEditor->setGeometry(option.rect);
        mEditor->update();
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
        else if(hoverEvent->type() == QEvent::Leave)
        {
            onHoverLeave(hoverEvent->index());
        }
    }

    return QStyledItemDelegate::event(event);
}

bool StalledIssueDelegate::eventFilter(QObject *object, QEvent *event)
{
    if(object == mView && (event->type() == QEvent::Resize
                           || event->type() == QEvent::Show))
    {
        mUpdateSizeHintTimerFromResize.start(UPDATE_SIZE_TIMER);

    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        if(auto mouseButtonEvent = dynamic_cast<QMouseEvent*>(event))
        {
            if(mouseButtonEvent->button() == Qt::LeftButton
                    && mouseButtonEvent->modifiers() == Qt::KeyboardModifier::NoModifier)
            {
                auto viewPos = mView->mapFromGlobal(mouseButtonEvent->globalPos());
                auto index = mView->indexAt(viewPos);

                if(index.isValid() && !index.parent().isValid())
                {
                    if(auto header = dynamic_cast<StalledIssueHeader*>(mEditor.data()))
                    {
                        if(header->isExpandable())
                        {
                            auto currentState = mView->isExpanded(index);
                            currentState ? mView->collapse(index) : mView->expand(index);

                            mEditor->expand(!currentState);

                            auto childIndex = index.model()->index(0,0,index);
                            //If it is going to be expanded
                            if(!currentState)
                            {
                                if(childIndex.isValid())
                                {
                                    mView->scrollTo(childIndex);
                                }
                            }

                            //As soon as the child is expanded,
                            //update the size as it may be the first time the widget is shown and it is outdated
                            QTimer::singleShot(0,[this, childIndex](){
                                sizeHintChanged(childIndex);
                            });

                            return true;
                        }
                    }
                }
            }
        }
    }

    return QStyledItemDelegate::eventFilter(object, event);
}

void StalledIssueDelegate::destroyEditor(QWidget*, const QModelIndex&) const
{
    //Do not destroy it the editor, as it is also used to paint the row and it is saved in a cache
    mEditor = nullptr;
}

void StalledIssueDelegate::onHoverEnter(const QModelIndex &index)
{
    QModelIndex editorCurrentIndex(getEditorCurrentIndex());

    if(editorCurrentIndex != index)
    {
        onHoverLeave(index);

        mView->edit(index);
    }
}

void StalledIssueDelegate::onHoverLeave(const QModelIndex& index)
{
    //It is mandatory to close the editor, as it may be different depending on the row
    if(mEditor)
    {
        closeEditor(mEditor, QAbstractItemDelegate::EndEditHint::NoHint);

        //Small hack to avoid blinks when changing from editor to delegate paint
        //Set the editor to nullptr and update the view -> Then the delegate paints the base widget
        //before the editor is removed
        mEditor = nullptr;
    }

    mView->update(index);
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

QModelIndex StalledIssueDelegate::getEditorCurrentIndex() const
{
    if(mEditor)
    {
       return mProxyModel->mapFromSource(mEditor->getCurrentIndex());
    }

    return QModelIndex();
}

StalledIssueBaseDelegateWidget *StalledIssueDelegate::getStalledIssueItemWidget(const QModelIndex &index, const StalledIssueVariant& data) const
{
    StalledIssueBaseDelegateWidget* item(nullptr);

    auto finalIndex(index);
    auto sourceIndex = mProxyModel->mapToSource(index);
    if(sourceIndex.isValid())
    {
        finalIndex = sourceIndex;
    }

    if(finalIndex.parent().isValid())
    {
        item = mCacheManager.getStalledIssueInfoWidget(finalIndex,mView->viewport(), data);
    }
    else
    {
        item = mCacheManager.getStalledIssueHeaderWidget(finalIndex,mView->viewport(), data);
    }

    return item;
}

#include "UserMessageDelegate.h"

#include "MegaDelegateHoverManager.h"
#include "UserMessage.h"
#include "UserMessageCacheManager.h"
#include "UserMessageProxyModel.h"
#include "UserMessageWidget.h"

#include <QPainter>
#include <QTimer>
#include <QTreeView>

#ifdef __APPLE__
#include <QScrollBar>
#endif

namespace
{
constexpr int UPDATE_DELAY = 50;
}

UserMessageDelegate::UserMessageDelegate(QAbstractItemModel* proxyModel, QTreeView* view):
    QStyledItemDelegate(view),
    mCacheManager(std::make_unique<UserMessageCacheManager>()),
    mEditor(std::make_unique<EditorInfo>()),
    mProxyModel(qobject_cast<UserMessageProxyModel*>(proxyModel)),
    mView(view)
{}

void UserMessageDelegate::paint(QPainter* painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    if(mEditor->getWidget() && mEditor->getIndex() == index)
    {
        return;
    }

    if (index.isValid())
    {
        painter->save();
        painter->translate(option.rect.topLeft());

        QWidget* item = getWidget(index);
        if(!item)
        {
            return;
        }

        item->resize(option.rect.width(), option.rect.height());
        item->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize UserMessageDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    QSize result;
    if (index.isValid())
    {
        QModelIndex filteredIndex = mProxyModel->mapToSource(index);
        if (filteredIndex.isValid() && filteredIndex.row() >= 0)
        {
            UserMessage* data = static_cast<UserMessage*>(filteredIndex.internalPointer());
            if(data)
            {
                result = data->sizeHint();
            }
        }

        if(result.isEmpty())
        {
            QWidget* item = getWidget(index);
            if(!item)
            {
                return QSize();
            }
            result = item->sizeHint();
        }
    }
    else
    {
        result = QStyledItemDelegate::sizeHint(option, index);
    }
    return result;
}

QWidget* UserMessageDelegate::createEditor(QWidget* parent,
                                           const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const
{
    Q_UNUSED(parent)
    Q_UNUSED(option)

    auto widget = getWidget(index);
    if(widget)
    {
        mEditor->setData(index, widget);
    }

    return mEditor->getWidget();
}

void UserMessageDelegate::destroyEditor(QWidget* editor,
                                        const QModelIndex& index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)

    //Do not destroy it the editor, as it is also used to paint the row and it is saved in a cache
    mEditor->setData(QModelIndex(), nullptr);
}

bool UserMessageDelegate::event(QEvent* event)
{
    if(auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        switch (static_cast<int>(hoverEvent->type()))
        {
            case MegaDelegateHoverEvent::Enter:
            case MegaDelegateHoverEvent::MouseMove:
            {
                onHoverEnter(hoverEvent->index());
                break;
            }
            case MegaDelegateHoverEvent::Leave:
            {
                onHoverLeave(hoverEvent->index());
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return QStyledItemDelegate::event(event);
}

void UserMessageDelegate::updateEditorGeometry(QWidget* editor,
                                               const QStyleOptionViewItem& option,
                                               const QModelIndex& index) const
{
    Q_UNUSED(index)

    QRect geometry(option.rect);
#ifdef __APPLE__
    auto width = mView->size().width();
    width -= mView->contentsMargins().left();
    width -= mView->contentsMargins().right();
    if(mView->verticalScrollBar() && mView->verticalScrollBar()->isVisible())
    {
        width -= mView->verticalScrollBar()->width();
    }
    geometry.setWidth(std::min(width, option.rect.width()));
#endif
    editor->setGeometry(geometry);
}

void UserMessageDelegate::onHoverEnter(const QModelIndex& index)
{
    QModelIndex editorCurrentIndex(mEditor->getIndex());
    if(editorCurrentIndex != index)
    {
        onHoverLeave(index);
        mView->edit(index);
    }
}

void UserMessageDelegate::onHoverLeave(const QModelIndex& index)
{
    //It is mandatory to close the editor, as it may be different depending on the row
    if(mEditor->getWidget())
    {
        emit closeEditor(mEditor->getWidget(), QAbstractItemDelegate::EndEditHint::NoHint);

        //Small hack to avoid blinks when changing from editor to delegate paint
        //Set the editor to nullptr and update the view -> Then the delegate paints the base widget
        //before the editor is removed
        mEditor->setData(QModelIndex(), nullptr);
    }

    QTimer::singleShot(UPDATE_DELAY, this, [this, index]()
    {
        mView->update(index);
    });
}

QModelIndex UserMessageDelegate::getEditorCurrentIndex() const
{
    if(mEditor)
    {
        return mProxyModel->mapFromSource(mEditor->getIndex());
    }

    return QModelIndex();
}

QWidget* UserMessageDelegate::getWidget(const QModelIndex& index) const
{
    UserMessageWidget* widget = nullptr;
    if (index.isValid() && index.row() >= 0)
    {
        QModelIndex filteredIndex = mProxyModel->mapToSource(index);
        if (filteredIndex.isValid() && filteredIndex.row() >= 0)
        {
            UserMessage* item = static_cast<UserMessage*>(filteredIndex.internalPointer());
            widget = mCacheManager->createOrGetWidget(index.row(), item, mView->viewport());
        }
    }
    return widget;
}

// ----------------------------------------------------------------------------
//      EditorInfo
// ----------------------------------------------------------------------------

UserMessageDelegate::EditorInfo::EditorInfo()
    : mIndex(QModelIndex())
    , mWidget(nullptr)
{
}

void UserMessageDelegate::EditorInfo::setData(const QModelIndex& index, QWidget* widget)
{
    mIndex = index;
    mWidget = widget;
}

QModelIndex UserMessageDelegate::EditorInfo::getIndex() const
{
    return mIndex;
}

QWidget* UserMessageDelegate::EditorInfo::getWidget() const
{
    return mWidget;
}

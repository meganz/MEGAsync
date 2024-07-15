#ifndef USER_MESSAGE_DELEGATE_H
#define USER_MESSAGE_DELEGATE_H

#include "NotificationDelegate.h"
#include "AlertDelegate.h"
#include "NotificationAlertTypes.h"

#include <QTreeView>
#include <QPointer>
#include <QStyledItemDelegate>

#include <memory>

class UserMessageProxyModel;

struct UserMessageEditorInfo
{
    UserMessageEditorInfo()
        : mIndex(QModelIndex())
        , mWidget(nullptr)
    {
    }

    void setData(const QModelIndex& index, QWidget* widget)
    {
        mIndex = index;
        mWidget = widget;
    }

    bool isEmpty() const
    {
        return !mWidget;
    }

    QModelIndex getIndex() const
    {
        return mIndex;
    }

    QWidget* getWidget() const
    {
        return mWidget;
    }

private:
    QModelIndex mIndex;
    QWidget* mWidget;

};

class UserMessageDelegate : public QStyledItemDelegate
{
public:
    UserMessageDelegate(QAbstractItemModel* proxyModel,
                              QTreeView* view);
    virtual ~UserMessageDelegate() = default;

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget *, const QModelIndex &) const override;
    bool event(QEvent* event) override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;

protected slots:
    void onHoverEnter(const QModelIndex& index);
    void onHoverLeave(const QModelIndex& index);

private:
    std::unique_ptr<AlertDelegate> mAlertsDelegate;
    std::unique_ptr<NotificationDelegate> mNotificationsDelegate;
    UserMessageProxyModel* mProxyModel;
    std::unique_ptr<UserMessageEditorInfo> mEditor;
    QTreeView* mView;

    QWidget* getWidget(const QModelIndex &index) const;
    QModelIndex getEditorCurrentIndex() const;

};

#endif // USER_MESSAGE_DELEGATE_H

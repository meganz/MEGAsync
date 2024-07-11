#ifndef NOTIFICATION_ALERT_DELEGATE_H
#define NOTIFICATION_ALERT_DELEGATE_H

#include "NotificationDelegate.h"
#include "AlertDelegate.h"
#include "NotificationAlertTypes.h"

#include <QTreeView>
#include <QPointer>
#include <QStyledItemDelegate>

#include <memory>

class NotificationAlertProxyModel;

struct NotificationEditorInfo
{
    NotificationEditorInfo()
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

class NotificationAlertDelegate : public QStyledItemDelegate
{
public:
    NotificationAlertDelegate(QAbstractItemModel* proxyModel,
                              QTreeView* view);
    virtual ~NotificationAlertDelegate() = default;

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void destroyEditor(QWidget *, const QModelIndex &) const override;
    bool event(QEvent* event) override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;

    //bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    //bool helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    //void createNotificationDelegate(NotificationModel* model);
    //void createAlertDelegate(AlertModel* model);

protected slots:
    void onHoverEnter(const QModelIndex& index);
    void onHoverLeave(const QModelIndex& index);

private:
    std::unique_ptr<AlertDelegate> mAlertsDelegate;
    std::unique_ptr<NotificationDelegate> mNotificationsDelegate;
    NotificationAlertProxyModel* mProxyModel;
    std::unique_ptr<NotificationEditorInfo> mEditor;
    QTreeView* mView;

    //NotificationAlertModelItem::ModelType getModelType(const QModelIndex& index) const;
    QWidget* getWidget(const QModelIndex &index) const;
    QModelIndex getEditorCurrentIndex() const;

};

#endif // NOTIFICATION_ALERT_DELEGATE_H

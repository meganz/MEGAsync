#ifndef USER_MESSAGE_DELEGATE_H
#define USER_MESSAGE_DELEGATE_H

#include <QTreeView>
#include <QStyledItemDelegate>

#include <memory>

class UserMessageProxyModel;
class UserMessageCacheManager;

class UserMessageDelegate : public QStyledItemDelegate
{

public:
    UserMessageDelegate() = delete;
    UserMessageDelegate(QAbstractItemModel* proxyModel, QTreeView* view);
    virtual ~UserMessageDelegate() = default;

protected:
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void destroyEditor(QWidget* editor,
                       const QModelIndex& index) const override;
    bool event(QEvent* event) override;
    void updateEditorGeometry(QWidget* editor,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

protected slots:
    void onHoverEnter(const QModelIndex& index);
    void onHoverLeave(const QModelIndex& index);

private:
    class EditorInfo
    {

    public:
        EditorInfo();
        ~EditorInfo() = default;

        void setData(const QModelIndex& index, QWidget* widget);
        QModelIndex getIndex() const;
        QWidget* getWidget() const;

    private:
        QModelIndex mIndex;
        QWidget* mWidget;

    };

    std::unique_ptr<UserMessageCacheManager> mCacheManager;
    std::unique_ptr<EditorInfo> mEditor;
    UserMessageProxyModel* mProxyModel;
    QTreeView* mView;

    QWidget* getWidget(const QModelIndex& index) const;
    QModelIndex getEditorCurrentIndex() const;

};

#endif // USER_MESSAGE_DELEGATE_H

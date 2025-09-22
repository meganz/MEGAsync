#ifndef NODESELECTORDELEGATES_H
#define NODESELECTORDELEGATES_H

#include <QHelpEvent>
#include <QStyledItemDelegate>

class NodeSelectorDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    NodeSelectorDelegate(QObject* parent);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool isHoverStateSet(const QModelIndex& index);

protected:
    void setPaintDevice(QPainter* painter, const QModelIndex& index) const;
    bool isPaintingDrag(QPainter* painter) const;
    bool event(QEvent* event) override;

private:
    mutable QPaintDevice* mMainDevice;
    static QModelIndex mLastHoverRow;
};

class NodeRowDelegate: public NodeSelectorDelegate
{
public:
    static const int MARGIN;
    static const int ICON_MARGIN;
    static const int DIFF_WITH_STD_ICON;

    explicit NodeRowDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event,
                   QAbstractItemView* view,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

#endif // NODESELECTORDELEGATES_H

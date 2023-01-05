#ifndef NODESELECTORDELEGATES_H
#define NODESELECTORDELEGATES_H

#include <QStyledItemDelegate>
#include <QHelpEvent>

class IconDelegate : public QStyledItemDelegate
{
    static const int ICON_HEIGHT;

public:
    explicit IconDelegate(QObject *parent = nullptr);
    ~IconDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
};

class NodeRowDelegate : public QStyledItemDelegate
{
public:
    static const int MARGIN;
    static const int ICON_MARGIN;
    static const int DIFF_WITH_STD_ICON;
    
    explicit NodeRowDelegate(QObject *parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;


    bool helpEvent(QHelpEvent *event,
                           QAbstractItemView *view,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) override;

private:
    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const override;
};

#endif // NODESELECTORDELEGATES_H

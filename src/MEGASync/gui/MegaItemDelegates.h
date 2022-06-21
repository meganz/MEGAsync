#ifndef MEGAITEMDELEGATES_H
#define MEGAITEMDELEGATES_H

#include <QStyledItemDelegate>

class IconDelegate : public QStyledItemDelegate
{
public:
    explicit IconDelegate(QObject *parent = nullptr);
    ~IconDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class NodeRowDelegate : public QStyledItemDelegate
{
public:
    static const int MARGIN;
    static const int ICON_MARGIN;
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

#endif // MEGAITEMDELEGATES_H

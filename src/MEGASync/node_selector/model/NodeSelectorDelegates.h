#ifndef NODESELECTORDELEGATES_H
#define NODESELECTORDELEGATES_H

#include <QAbstractItemView>
#include <QHash>
#include <QHelpEvent>
#include <QStyledItemDelegate>

#include <memory>

class QTextDocument;

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
    bool event(QEvent* event) override;
    virtual QColor textColorForIndex(const QModelIndex& index, bool isTakenDown) const;

    virtual void adjustContentRect(QStyleOptionViewItem* option, const QModelIndex& index) const {}

private:
    // Persistent so it is invalidated by the proxy when it rebuilds its mappings
    // (invalidateFilter). A plain QModelIndex would dangle and crash if painted afterwards.
    QPersistentModelIndex mLastHoverRow;
};

class NodeRowDelegate: public NodeSelectorDelegate
{
public:
    static const int MARGIN;
    static const int ICON_MARGIN;
    static const int DIFF_WITH_STD_ICON;
    static const int ROW_HEIGHT;
    static const int IS_EXPORTED_RIGHT_MARGIN;

    explicit NodeRowDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QPixmap paintForDrag(const QModelIndex& index, QAbstractItemView* view) const;
    // Composite drag pixmap: stacks the selected rows (capped) with a "+N" badge.
    QPixmap paintForDrag(const QModelIndexList& rows, QAbstractItemView* view) const;

    bool helpEvent(QHelpEvent* event,
                   QAbstractItemView* view,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

private:
    void paintDragOverflowBadge(QPainter& painter, const QRect& pixmapRect, int hiddenRows) const;
};

class NodeLabelDelegate: public NodeSelectorDelegate
{
public:
    explicit NodeLabelDelegate(bool showLabelText, QObject* parent = nullptr);

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool helpEvent(QHelpEvent* event,
                   QAbstractItemView* view,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

private:
    bool mShowLabelText = true;
    mutable bool mSuppressText = false;
};

class NodeSearchRowDelegate: public NodeRowDelegate
{
    Q_OBJECT

public:
    explicit NodeSearchRowDelegate(QObject* parent = nullptr);
    void setSearchText(const QString& text);
    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

private:
    static QString buildHighlightedHtml(const QString& display, const QString& search);

    QString mSearchText;
    mutable bool mSuppressText = false;
    mutable QHash<QString, std::shared_ptr<QTextDocument>> mDocumentCache;
};

#endif // NODESELECTORDELEGATES_H

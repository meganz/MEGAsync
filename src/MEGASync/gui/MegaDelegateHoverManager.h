#ifndef MEGADELEGATEHOVERMANAGER_H
#define MEGADELEGATEHOVERMANAGER_H

#include <QEvent>
#include <QStyledItemDelegate>
#include <QAbstractItemView>

class MegaDelegateHoverEvent : public QEvent
{
public:
    MegaDelegateHoverEvent(QEvent::Type type):QEvent(type) {}

    QModelIndex index() const{return mIndex;}
    void setIndex(const QModelIndex &index){ mIndex = index;}

    QRect rect() const {return mRect;}
    void setRect(const QRect &rect) {mRect = rect;}

    QPoint mousePos() const {return mMousePos;}
    void setMousePos(const QPoint &mousePos) {mMousePos = mousePos;}

private:
    QModelIndex mIndex;
    QRect mRect;
    QPoint mMousePos;
};

class MegaDelegateHoverManager : public QObject
{
    Q_OBJECT

public:
    MegaDelegateHoverManager();
    void setView(QAbstractItemView* view);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QAbstractItemView* mView;
    QStyledItemDelegate* mDelegate;
    QPersistentModelIndex mCurrentIndex;

    void sendEvent(QEvent::Type eventType, const QPoint& point = QPoint());
};


#endif // MEGADELEGATEHOVERMANAGER_H

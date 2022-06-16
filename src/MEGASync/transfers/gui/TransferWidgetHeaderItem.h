#ifndef TRANSFERWIDGETHEADERITEM_H
#define TRANSFERWIDGETHEADERITEM_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class TransferWidgetHeaderItem;
}

class TransferWidgetHeaderItem : public QWidget
{
    Q_OBJECT

public:
    explicit TransferWidgetHeaderItem(QWidget *parent = nullptr);
    ~TransferWidgetHeaderItem();

    Q_PROPERTY(QString title MEMBER mTitle READ title() WRITE setTitle())
    QString title() const;
    void setTitle(const QString &title);

    Q_PROPERTY(int sortCriterion MEMBER mSortCriterion READ sortCriterion() WRITE setSortCriterion())
    int sortCriterion() const;
    void setSortCriterion(const int &sortCriterion);

    void setSortOrder(Qt::SortOrder order);
    void turnOffSorting();

signals:
    void toggled(int, Qt::SortOrder);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Ui::TransferWidgetHeaderItem *ui;
    Qt::SortOrder mCurrentSortOrder;
    int mSortCriterion;
    QString mTitle;

    void updateChevronIcon();
    void turnOffSiblings();
    bool isTurnedOff();
};

#endif // TRANSFERWIDGETHEADERITEM_H

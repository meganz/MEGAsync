#ifndef STALLEDISSUEBASEDELEGATEWIDGET_H
#define STALLEDISSUEBASEDELEGATEWIDGET_H

#include <StalledIssue.h>

#include <QWidget>
#include <QStyleOptionViewItem>

class StalledIssueBaseDelegateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StalledIssueBaseDelegateWidget(QWidget *parent = nullptr);

    void render(const QStyleOptionViewItem &option,
                QPainter *painter,
                const QRegion &sourceRegion);
    virtual void updateUi(const QModelIndex &index, const StalledIssue &data);

    QModelIndex getCurrentIndex() const;

    const StalledIssue &getData() const;

signals:
    void issueFixed();

protected:
    virtual void refreshUi() = 0;

private:
    StalledIssue mData;
    QModelIndex mCurrentIndex;
};

#endif // STALLEDISSUEBASEDELEGATEWIDGET_H

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
    virtual void updateUi(const QModelIndex &index, const QExplicitlySharedDataPointer<StalledIssueData> data);

    QModelIndex getCurrentIndex() const;

    QExplicitlySharedDataPointer<StalledIssueData> getData() const;

protected:
    virtual void refreshUi() = 0;

private:
    QExplicitlySharedDataPointer<StalledIssueData> mData;
    QModelIndex mCurrentIndex;
};

#endif // STALLEDISSUEBASEDELEGATEWIDGET_H

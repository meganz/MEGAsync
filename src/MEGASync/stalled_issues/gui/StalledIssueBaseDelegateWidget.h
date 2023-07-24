#ifndef STALLEDISSUEBASEDELEGATEWIDGET_H
#define STALLEDISSUEBASEDELEGATEWIDGET_H

#include <StalledIssue.h>
#include <StalledIssuesUtilities.h>

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTimer>


class StalledIssueBaseDelegateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StalledIssueBaseDelegateWidget(QWidget *parent);

    void updateIndex();
    virtual void expand(bool){}

    void render(const QStyleOptionViewItem &option,
                QPainter *painter,
                const QRegion &sourceRegion);
    virtual void updateUi(const QModelIndex &index, const StalledIssueVariant& issueData);
    virtual void setIndent(int){}

    QModelIndex getCurrentIndex() const;
    const StalledIssueVariant &getData() const;

    virtual void reset();
    QSize sizeHint() const override;

    bool isHeader() const;

    void setDelegate(QStyledItemDelegate *newDelegate);

    void updateSizeHint();

signals:
    void editorKeepStateChanged(bool state);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

    StalledIssuesUtilities mUtilities;
    QStyledItemDelegate* mDelegate;

private slots:
    void checkForSizeHintChanges();

private:
    virtual void refreshUi() = 0;
    QTimer mSizeHintTimer;
    uint8_t mSizeHintChanged = 0;
    int mLastSizeHint = 0;

    mutable StalledIssueVariant mData;
    QPersistentModelIndex mCurrentIndex;
};

#endif // STALLEDISSUEBASEDELEGATEWIDGET_H

#ifndef STALLEDISSUEBASEDELEGATEWIDGET_H
#define STALLEDISSUEBASEDELEGATEWIDGET_H

#include "MessageDialogOpener.h"
#include "StalledIssue.h"
#include "StalledIssuesUtilities.h"

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QTimer>
#include <QWidget>

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

    bool checkForExternalChanges(bool isSingleSelection);

signals:
    void editorKeepStateChanged(bool state);
    void needsUpdate();

protected:
    struct SelectionInfo
    {
        QModelIndexList selection;
        QModelIndexList similarSelection;
        MessageDialogInfo msgInfo;
    };
    bool checkSelection(const QList<mega::MegaSyncStall::SyncStallReason>& reasons, SelectionInfo& info);

    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

    StalledIssuesUtilities mUtilities;
    QStyledItemDelegate* mDelegate;

private slots:
    void checkForSizeHintChanges();

private:
    virtual void refreshUi() = 0;

    mutable StalledIssueVariant mData;
    QPersistentModelIndex mCurrentIndex;
    QTimer mResizeNeedTimer;
};

#endif // STALLEDISSUEBASEDELEGATEWIDGET_H

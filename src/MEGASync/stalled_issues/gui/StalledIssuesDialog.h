#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include "MegaDelegateHoverManager.h"
#include "StalledIssue.h"

#include <QDialog>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssueTab;
class StalledIssuesProxyModel;
class StalledIssueDelegate;
class SyncSettings;

class StalledIssuesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

    QModelIndexList getSelection(QList<mega::MegaSyncStall::SyncStallReason> reasons) const;
    QModelIndexList getSelection(std::function<bool (const std::shared_ptr<const StalledIssue>)> checker) const;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool event(QEvent* event) override;

private slots:
    void on_doneButton_clicked();
    void on_refreshButton_clicked();
    void checkIfViewIsEmpty();

    void onSyncRootChanged(std::shared_ptr<SyncSettings> sync);
    void onGlobalSyncStateChanged(bool);

    void onTabToggled(StalledIssueFilterCriterion filterCriterion);
    bool toggleTabAndScroll(StalledIssueFilterCriterion filterCriterion, const QModelIndex& sourceIndex);

    void onUiBlocked();
    void onUiUnblocked();

    void onStalledIssuesLoaded();
    void onModelFiltered();
    void onLoadingSceneVisibilityChange(bool state);

    void onScrollRangeChanged(int, int max);

private:
    void showView();
    void setScrollMode(bool state);

    Ui::StalledIssuesDialog *ui;
    MegaDelegateHoverManager mViewHoverManager;
    StalledIssueFilterCriterion mCurrentTab;
    StalledIssuesProxyModel* mProxyModel;
    StalledIssueDelegate* mDelegate;

};

#endif // STALLEDISSUESDIALOG_H

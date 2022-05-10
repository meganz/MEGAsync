#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include "MegaDelegateHoverManager.h"
#include "StalledIssue.h"
#include "StalledIssueLoadingItem.h"

#include <ViewLoadingScene.h>

#include <QDialog>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssueTab;
class StalledIssuesProxyModel;

class StalledIssuesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

protected:
    bool eventFilter(QObject *, QEvent *) override;

private slots:
    void on_doneButton_clicked();
    void on_updateButton_clicked();
    void onStalledIssuesModelCountChanged();
    void onGlobalSyncStateChanged(bool state);

    void toggleTab(StalledIssueFilterCriterion filterCriterion);

    void onUiBlocked();
    void onUiUnblocked();

    void onStalledIssuesLoaded();

private:
    Ui::StalledIssuesDialog *ui;
    MegaDelegateHoverManager mViewHoverManager;
    StalledIssueFilterCriterion mCurrentTab;
    ViewLoadingScene<StalledIssueLoadingItem> mLoadingScene;
    StalledIssuesProxyModel* mProxyModel;
};

#endif // STALLEDISSUESDIALOG_H

#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include "MegaDelegateHoverManager.h"
#include "StalledIssue.h"
#include "StalledIssueLoadingItem.h"
#include "Preferences.h"

#include <ViewLoadingScene.h>

#include <QDialog>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssueTab;
class StalledIssuesProxyModel;
class StalledIssueDelegate;

class StalledIssuesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

    QModelIndexList getSelection(QList<mega::MegaSyncStall::SyncStallReason> reasons) const;
    QModelIndexList getSelection(std::function<bool (const std::shared_ptr<const StalledIssue>)> checker) const;

protected:
    bool eventFilter(QObject *, QEvent *) override;

private slots:
    void on_doneButton_clicked();
    void on_updateButton_clicked();
    void checkIfViewIsEmpty();
    void onGlobalSyncStateChanged(bool);

    void toggleTab(StalledIssueFilterCriterion filterCriterion);

    void onUiBlocked();
    void onUiUnblocked();

    void onStalledIssuesLoaded();
    void onModelFiltered();
    void onLoadingSceneChanged(bool state);

    void showModeSelector();
    void onPreferencesValueChanged(QString key);

private:
    void showView(bool update);
    void selectNewMode();

    Ui::StalledIssuesDialog *ui;
    MegaDelegateHoverManager mViewHoverManager;
    StalledIssueFilterCriterion mCurrentTab;
    StalledIssuesProxyModel* mProxyModel;
    StalledIssueDelegate* mDelegate;

    Preferences::StalledIssuesModeType mModeSelected = Preferences::StalledIssuesModeType::None;
};

#endif // STALLEDISSUESDIALOG_H

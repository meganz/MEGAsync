#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include "MegaDelegateHoverManager.h"

#include <QDialog>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssuesDialog : public QDialog
{
    Q_OBJECT
public:
    enum SI_TAB
    {
        ALL_ISSUES_TAB         = 0,
        NAME_CONFLICTS_TAB     = 1,
        ITEM_TYPE_TAB          = 2,
        OTHER_TAB              = 3
    };
    Q_ENUM(SI_TAB)

    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

protected:
    bool eventFilter(QObject *, QEvent *) override;

private slots:
    void on_doneButton_clicked();
    void on_updateButton_clicked();
    void onStalledIssuesModelCountChanged();

    void toggleTab(QObject* toggledObj);

private:
    Ui::StalledIssuesDialog *ui;
    MegaDelegateHoverManager mViewHoverManager;
    SI_TAB mCurrentTab;

    QMap<SI_TAB, QWidget*> mTabFramesToggleGroup;
    QGraphicsDropShadowEffect* mShadowTab;
};

#endif // STALLEDISSUESDIALOG_H

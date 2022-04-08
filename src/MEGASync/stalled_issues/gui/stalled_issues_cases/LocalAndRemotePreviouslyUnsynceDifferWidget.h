#ifndef LOCALANDREMOTEPREVIOUSLYUNSYNCEDIFFERWIDGET_H
#define LOCALANDREMOTEPREVIOUSLYUNSYNCEDIFFERWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>

namespace Ui {
class LocalAndRemotePreviouslyUnsynceDifferWidget;
}

class LocalAndRemotePreviouslyUnsynceDifferWidget : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit LocalAndRemotePreviouslyUnsynceDifferWidget(QWidget *parent = nullptr);
    ~LocalAndRemotePreviouslyUnsynceDifferWidget();

    void refreshUi() override;

private slots:
    void onLocalButtonClicked();
    void onRemoteButtonClicked();

private:
    Ui::LocalAndRemotePreviouslyUnsynceDifferWidget *ui;
};

#endif // LOCALANDREMOTEPREVIOUSLYUNSYNCEDIFFERWIDGET_H

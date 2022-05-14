#ifndef LOCALANDREMOTEDIFFERENTWIDGET_H
#define LOCALANDREMOTEDIFFERENTWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"
#include "QTMegaRequestListener.h"

#include <QWidget>
#include <memory>

namespace Ui {
class LocalAndRemoteDifferentWidget;
}

class LocalAndRemoteDifferentWidget : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit LocalAndRemoteDifferentWidget(QWidget *parent = nullptr);
    ~LocalAndRemoteDifferentWidget();

    void refreshUi() override;

private slots:
    void onLocalButtonClicked(int);
    void onRemoteButtonClicked(int);

private:
    Ui::LocalAndRemoteDifferentWidget *ui;
};

#endif // LOCALANDREMOTEDIFFERENTWIDGET_H

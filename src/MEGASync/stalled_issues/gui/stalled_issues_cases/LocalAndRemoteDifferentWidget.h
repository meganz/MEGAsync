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
    explicit LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> orignalstall, QWidget *parent = nullptr);
    ~LocalAndRemoteDifferentWidget();

    void refreshUi() override;

    std::shared_ptr<mega::MegaSyncStall> originalStall;

private slots:
    void onLocalButtonClicked(int);
    void onRemoteButtonClicked(int);

private:
    bool checkIssue(QDialog* dialog);

    Ui::LocalAndRemoteDifferentWidget *ui;
};

#endif // LOCALANDREMOTEDIFFERENTWIDGET_H

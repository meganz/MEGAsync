#ifndef LOCALANDREMOTEDIFFERENTWIDGET_H
#define LOCALANDREMOTEDIFFERENTWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"

#include <memory>

namespace Ui {
class LocalAndRemoteDifferentWidget;
}

class StalledIssueChooseWidget;

class LocalAndRemoteDifferentWidget : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> originalStall, QWidget *parent = nullptr);
    ~LocalAndRemoteDifferentWidget();

    void refreshUi() override;

    std::shared_ptr<mega::MegaSyncStall> originalStall;

    //Commong strings for Local/Remote selection
    struct KeepSideInfo
    {
        bool isFile = false;
        int numberOfIssues = 1;
        QString itemName;
    };

    static QString keepLocalSideString(const KeepSideInfo& info);
    static QString keepRemoteSideString(const KeepSideInfo& info);

private slots:
    void onLocalButtonClicked(int);
    void onRemoteButtonClicked(int);
    void onKeepBothButtonClicked(int);
    void onKeepLastModifiedTimeButtonClicked(int);

private:
    void unSetFailedChooseWidget();
    std::unique_ptr<mega::MegaNode> getNode();

    Ui::LocalAndRemoteDifferentWidget *ui;
    StalledIssueChooseWidget* mFailedItem;
};

#endif // LOCALANDREMOTEDIFFERENTWIDGET_H

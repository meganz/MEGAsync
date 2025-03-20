#include "MoveOrRenameCannotOccur.h"

#include "LocalAndRemoteDifferentWidget.h"
#include "MegaApplication.h"
#include "MoveOrRenameCannotOccurIssue.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesModel.h"
#include "TextDecorator.h"
#include "ui_MoveOrRenameCannotOccur.h"

#include <QDialogButtonBox>

namespace
{
Text::Bold boldTextDecorator;
const Text::Decorator textDecorator(&boldTextDecorator);
}

MoveOrRenameCannotOccur::MoveOrRenameCannotOccur(QWidget* parent)
    : StalledIssueBaseDelegateWidget(parent)
    , ui(new Ui::MoveOrRenameCannotOccur)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &MoveOrRenameCannotOccur::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &MoveOrRenameCannotOccur::onRemoteButtonClicked);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->chooseLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::BODY_INDENT);
}

MoveOrRenameCannotOccur::~MoveOrRenameCannotOccur()
{
    delete ui;
}

void MoveOrRenameCannotOccur::refreshUi()
{
    auto issue = getData().convert<MoveOrRenameCannotOccurIssue>();
    ui->chooseLocalCopy->updateUi(issue);
    ui->chooseLocalCopy->show();

    ui->chooseRemoteCopy->updateUi(issue);
    ui->chooseRemoteCopy->show();
}

void MoveOrRenameCannotOccur::onLocalButtonClicked()
{
    SelectionInfo info;
    if(!checkSelection(QList<mega::MegaSyncStall::SyncStallReason>()
                           << mega::MegaSyncStall::MoveOrRenameCannotOccur,info))
    {
        return;
    }

    if(info.similarSelection.size() > 1)
    {
        LocalAndRemoteDifferentWidget::KeepSideInfo stringInfo;
        stringInfo.numberOfIssues = info.selection.size();
        stringInfo.itemName = getData().convert<MoveOrRenameCannotOccurIssue>()->syncName();
        info.msgInfo.titleText = LocalAndRemoteDifferentWidget::keepLocalSideString(stringInfo);
        textDecorator.process(info.msgInfo.titleText);

        info.msgInfo.finishFunc = [info](QPointer<MessageDialogResult> msgBox)
        {
            if (msgBox->result() == QMessageBox::Ok)
            {
                if (msgBox->isChecked())
                {
                    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(
                        info.similarSelection, MoveOrRenameIssueChosenSide::LOCAL);
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(
                        info.selection, MoveOrRenameIssueChosenSide::LOCAL);
                }
            }
        };

        MessageDialogOpener::warning(info.msgInfo);
    }
    else
    {
        MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(
            QModelIndexList() << getCurrentIndex(), MoveOrRenameIssueChosenSide::LOCAL);
    }
}

void MoveOrRenameCannotOccur::onRemoteButtonClicked()
{
    SelectionInfo info;
    if(!checkSelection(QList<mega::MegaSyncStall::SyncStallReason>()
                           << mega::MegaSyncStall::MoveOrRenameCannotOccur, info))
    {
        return;
    }

    if(info.similarSelection.size() > 1)
    {
        LocalAndRemoteDifferentWidget::KeepSideInfo stringInfo;
        stringInfo.numberOfIssues = info.selection.size();
        stringInfo.itemName = getData().convert<MoveOrRenameCannotOccurIssue>()->syncName();
        info.msgInfo.titleText = LocalAndRemoteDifferentWidget::keepRemoteSideString(stringInfo);
        textDecorator.process(info.msgInfo.titleText);

        info.msgInfo.finishFunc = [info](QPointer<MessageDialogResult> msgBox)
        {
            if (msgBox->result() == QMessageBox::Ok)
            {
                if (msgBox->isChecked())
                {
                    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(
                        info.similarSelection, MoveOrRenameIssueChosenSide::REMOTE);
                }
                else
                {
                    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(
                        info.selection, MoveOrRenameIssueChosenSide::REMOTE);
                }
            }
        };

        MessageDialogOpener::warning(info.msgInfo);
    }
    else
    {
        MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(
            QModelIndexList() << getCurrentIndex(), MoveOrRenameIssueChosenSide::REMOTE);
    }
}

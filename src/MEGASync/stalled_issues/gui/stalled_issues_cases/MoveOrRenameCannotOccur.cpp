#include "MoveOrRenameCannotOccur.h"
#include "ui_MoveOrRenameCannotOccur.h"

#include <StalledIssueHeader.h>
#include <MoveOrRenameCannotOccurIssue.h>
#include <MegaApplication.h>
#include <StalledIssuesModel.h>

#include <TextDecorator.h>

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
    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(getCurrentIndex(), MoveOrRenameCannotOccurIssue::ChosenSide::LOCAL);
}

void MoveOrRenameCannotOccur::onRemoteButtonClicked()
{
    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(getCurrentIndex(), MoveOrRenameCannotOccurIssue::ChosenSide::REMOTE);
}

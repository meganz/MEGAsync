#include "MoveOrRenameCannotOccur.h"
#include "ui_MoveOrRenameCannotOccur.h"

#include <StalledIssueHeader.h>
#include <MoveOrRenameCannotOccurIssue.h>

MoveOrRenameCannotOccur::MoveOrRenameCannotOccur(QWidget* parent)
    : StalledIssueBaseDelegateWidget(parent)
    , ui(new Ui::MoveOrRenameCannotOccur)
{
    ui->setupUi(this);

    //connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &MoveOrRenameCannotOccur::onLocalButtonClicked);
    //connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &MoveOrRenameCannotOccur::onRemoteButtonClicked);

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

    if(issue->consultLocalData())
    {
        //ui->chooseLocalCopy->updateUi(issue->consultLocalData(), issue->getChosenSide());

        ui->chooseLocalCopy->show();
    }
    else
    {
        ui->chooseLocalCopy->hide();
    }

    if(issue->consultCloudData())
    {
        //ui->chooseRemoteCopy->updateUi(issue->consultCloudData(), issue->getChosenSide());

        ui->chooseRemoteCopy->show();
    }
    else
    {
        ui->chooseRemoteCopy->hide();
    }
}

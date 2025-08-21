#include "MoveOrRenameCannotOccurChooseWidget.h"

#include "MegaApplication.h"
#include "ui_StalledIssueChooseWidget.h"

//BASE CLASS
MoveOrRenameCannotOccurChooseWidget::MoveOrRenameCannotOccurChooseWidget(QWidget* parent):
    StalledIssueChooseWidget(parent),
    mChosenSide(MoveOrRenameIssueChosenSide::NONE)
{}

void MoveOrRenameCannotOccurChooseWidget::updateUi(
    std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    ui->chooseTitle->showIcon();

    ui->name->setHyperLinkMode();
    ui->name->setIsFile(issue->isFile());
    ui->name->setInfo(QString(), mega::INVALID_HANDLE);
    ui->name->showIcon();

    ui->pathContainer->hide();

    auto side(issue->getChosenSide());

    if(issue->isFailed())
    {
        ui->chooseTitle->setActionButtonVisibility(StalledIssueChooseWidget::BUTTON_ID, true);
    }
    else if(!issue->isUnsolved() && side != MoveOrRenameIssueChosenSide::NONE)
    {
        ui->chooseTitle->setActionButtonVisibility(StalledIssueChooseWidget::BUTTON_ID, false);

        if(side == mChosenSide)
        {
            ui->chooseTitle->setMessage(
                chosenString(),
                Utilities::getPixmapName(QLatin1String("check_support_success"),
                                         Utilities::AttributeType::NONE));
        }
        else
        {
            ui->chooseTitle->setMessage(
                tr("Changes Undone"),
                Utilities::getPixmapName(QLatin1String("rotate_arrow_support_error"),
                                         Utilities::AttributeType::NONE));
        }
    }

    update();

    mData = issue;
}


//LOCAL CLASS
LocalMoveOrRenameCannotOccurChooseWidget::LocalMoveOrRenameCannotOccurChooseWidget(QWidget *parent) :
    MoveOrRenameCannotOccurChooseWidget(parent)
{
    mChosenSide = MoveOrRenameIssueChosenSide::LOCAL;
}

void LocalMoveOrRenameCannotOccurChooseWidget::updateUi(
    std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    MoveOrRenameCannotOccurChooseWidget::updateUi(issue);
    ui->chooseTitle->setHTML(tr("Local"));
    ui->name->setIsCloud(false);


    if(!issue->isUnsolved() && issue->getChosenSide() != MoveOrRenameIssueChosenSide::NONE)
    {
        setSolved(true, issue->getChosenSide() == MoveOrRenameIssueChosenSide::LOCAL);
    }
    else
    {
        if (issue->isKeepSideAvailable(MoveOrRenameIssueChosenSide::LOCAL))
        {
            addDefaultButton();
        }

        setSolved(false, false);
    }

    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(mData->firstSyncId()));
    if(sync)
    {
        QString pathStr(QString::fromUtf8(sync->getLocalFolder()));
        ui->name->setHTML(pathStr);
        ui->name->setInfo(pathStr, mega::INVALID_HANDLE);
    }
}

//REMOTE CLASS
RemoteMoveOrRenameCannotOccurChooseWidget::RemoteMoveOrRenameCannotOccurChooseWidget(QWidget *parent) :
    MoveOrRenameCannotOccurChooseWidget(parent)
{
     mChosenSide = MoveOrRenameIssueChosenSide::REMOTE;
}

void RemoteMoveOrRenameCannotOccurChooseWidget::updateUi(
    std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    MoveOrRenameCannotOccurChooseWidget::updateUi(issue);
    ui->chooseTitle->setHTML(tr("Remote"));
    ui->name->setIsCloud(true);


    if(!issue->isUnsolved() && issue->getChosenSide() != MoveOrRenameIssueChosenSide::NONE)
    {
        setSolved(true, issue->getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE);
    }
    else
    {
        if (issue->isKeepSideAvailable(MoveOrRenameIssueChosenSide::REMOTE))
        {
            addDefaultButton();
        }

        setSolved(false, false);
    }

    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(mData->firstSyncId()));
    if(sync)
    {
        std::unique_ptr<mega::MegaNode> remoteFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(sync->getMegaHandle()));
        std::unique_ptr<const char[]> path(MegaSyncApp->getMegaApi()->getNodePath(remoteFolder.get()));
        QString pathStr(QString::fromUtf8(path.get()));
        ui->name->setInfo(pathStr, mega::INVALID_HANDLE);
        ui->name->setHTML(QString::fromUtf8(path.get()));
    }
}

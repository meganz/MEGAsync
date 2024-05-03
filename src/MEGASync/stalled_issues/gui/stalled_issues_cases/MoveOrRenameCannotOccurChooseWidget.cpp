#include "MoveOrRenameCannotOccurChooseWidget.h"

#include "ui_StalledIssueChooseWidget.h"
#include "syncs/control/SyncController.h"

//BASE CLASS
MoveOrRenameCannotOccurChooseWidget::MoveOrRenameCannotOccurChooseWidget(QWidget *parent) :
    mSide(MoveOrRenameCannotOccurIssue::ChosenSide::NONE),
    StalledIssueChooseWidget(parent)
{
}

MoveOrRenameCannotOccurChooseWidget::~MoveOrRenameCannotOccurChooseWidget()
{
}

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

    if(!isSolved() && side != MoveOrRenameCannotOccurIssue::ChosenSide::NONE)
    {
        ui->chooseTitle->hideActionButton(StalledIssueChooseWidget::BUTTON_ID);

        QIcon icon;
        if(side == mSide)
        {
            icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
            ui->chooseTitle->setMessage(tr("Chosen"), icon.pixmap(16, 16));
        }
        else
        {
            icon.addFile(QString::fromUtf8(":/images/StalledIssues/rotate-ccw.png"));
            ui->chooseTitle->setMessage(tr("Changes Undone"), icon.pixmap(16, 16));
        }
    }

    update();

    mData = issue;
}


//LOCAL CLASS
LocalMoveOrRenameCannotOccurChooseWidget::LocalMoveOrRenameCannotOccurChooseWidget(QWidget *parent) :
    MoveOrRenameCannotOccurChooseWidget(parent)
{
    mSide = MoveOrRenameCannotOccurIssue::ChosenSide::LOCAL;
}

LocalMoveOrRenameCannotOccurChooseWidget::~LocalMoveOrRenameCannotOccurChooseWidget() {}

void LocalMoveOrRenameCannotOccurChooseWidget::updateUi(
    std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    MoveOrRenameCannotOccurChooseWidget::updateUi(issue);
    ui->chooseTitle->setHTML(tr("Local"));
    ui->name->setIsCloud(false);

    if(issue->isSolved())
    {
        setSolved(issue->getChosenSide() != MoveOrRenameCannotOccurIssue::ChosenSide::LOCAL);
    }

    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(mData->syncIds().first()));
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
     mSide = MoveOrRenameCannotOccurIssue::ChosenSide::REMOTE;
}

RemoteMoveOrRenameCannotOccurChooseWidget::~RemoteMoveOrRenameCannotOccurChooseWidget() {}

void RemoteMoveOrRenameCannotOccurChooseWidget::updateUi(
    std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    MoveOrRenameCannotOccurChooseWidget::updateUi(issue);
    ui->chooseTitle->setHTML(tr("Remote"));
    ui->name->setIsCloud(true);

    if(issue->isSolved())
    {
        setSolved(issue->getChosenSide() != MoveOrRenameCannotOccurIssue::ChosenSide::REMOTE);
    }

    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(mData->syncIds().first()));
    if(sync)
    {
        std::unique_ptr<mega::MegaNode> remoteFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(sync->getMegaHandle()));
        std::unique_ptr<const char[]> path(MegaSyncApp->getMegaApi()->getNodePath(remoteFolder.get()));
        QString pathStr(QString::fromUtf8(path.get()));
        ui->name->setInfo(pathStr, mega::INVALID_HANDLE);
        ui->name->setHTML(QString::fromUtf8(path.get()));
    }
}

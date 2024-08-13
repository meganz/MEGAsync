#include "FolderMatchedAgainstFileWidget.h"
#include "TextDecorator.h"
#include <QDialogButtonBox>
#include "ui_FolderMatchedAgainstFileWidget.h"
#include <QCheckBox>

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <PlatformStrings.h>
#include <QMegaMessageBox.h>
#include "FolderMatchedAgainstFileIssue.h"
#include "StalledIssueChooseWidget.h"
#include <Preferences/Preferences.h>

#include "mega/types.h"

#include <QMessageBox>
#include <QFile>


FolderMatchedAgainstFileWidget::FolderMatchedAgainstFileWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::FolderMatchedAgainstFileWidget)
{
    ui->setupUi(this);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->chooseLayout->setContentsMargins(margins);
}

FolderMatchedAgainstFileWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void FolderMatchedAgainstFileWidget::refreshUi()
{
    const auto issue = getData().convert<FolderMatchedAgainstFileIssue>();
    const auto isFailed(issue->isFailed());
    const auto chosenSide(issue->getResult().sideRenamed);

    if(issue->consultLocalData())
    {
        ui->chooseLocalCopy->updateUi(issue->consultLocalData(), chosenSide);

        ui->chooseLocalCopy->show();
    }
    else
    {
        ui->chooseLocalCopy->hide();
    }

    if(issue->consultCloudData())
    {
        ui->chooseRemoteCopy->updateUi(issue->consultCloudData(), chosenSide);
        ui->chooseRemoteCopy->show();
    }
    else
    {
        ui->chooseRemoteCopy->hide();
    }

    if(issue->getSyncType() != mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        GenericChooseWidget::GenericInfo bothInfo;
        bothInfo.buttonText = tr("Choose both");
        QString bothInfoTitle = tr("[B]Keep both[/B]");
        StalledIssuesBoldTextDecorator::boldTextDecorator.process(bothInfoTitle);
        bothInfo.title = bothInfoTitle;
        bothInfo.icon = QLatin1String(":/images/copy.png");
        bothInfo.solvedText = ui->keepBothOption->chosenString();
        ui->keepBothOption->setInfo(bothInfo);

        GenericChooseWidget::GenericInfo lastModifiedInfo;
        lastModifiedInfo.buttonText = tr("Choose");
        QString lastModifiedInfoTitle;
        if(issue->lastModifiedSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL)
        {
            lastModifiedInfoTitle = tr("[B]Keep last modified[/B] (local)");
        }
        else
        {
            lastModifiedInfoTitle = tr("[B]Keep last modified[/B] (remote)");
        }

        StalledIssuesBoldTextDecorator::boldTextDecorator.process(lastModifiedInfoTitle);
        lastModifiedInfo.title = lastModifiedInfoTitle;
        lastModifiedInfo.icon = QLatin1String(":/images/clock_ico.png");
        lastModifiedInfo.solvedText = ui->keepLastModifiedOption->chosenString();
        ui->keepLastModifiedOption->setInfo(lastModifiedInfo);
    }
    else
    {
        ui->chooseRemoteCopy->setActionButtonVisibility(false);
        ui->keepBothOption->hide();
        ui->keepLastModifiedOption->hide();
    }

    if (issue->isSolved())
    {
        unSetFailedChooseWidget();

        ui->keepBothOption->setSolved(true, issue->getChosenSide() == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::BOTH);
        ui->keepLastModifiedOption->hide();

        if (issue->isPotentiallySolved())
        {
            ui->chooseLocalCopy->setActionButtonVisibility(false);
            ui->chooseRemoteCopy->setActionButtonVisibility(false);
        }

    }
    else if(issue->isFailed())
    {
        ui->keepBothOption->setSolved(false, false);
        ui->keepLastModifiedOption->show();
        ui->chooseLocalCopy->setActionButtonVisibility(true);
        ui->chooseRemoteCopy->setActionButtonVisibility(true);

        unSetFailedChooseWidget();

        switch(issue->getChosenSide())
        {
            case LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::REMOTE:
            {
                auto errorStr = issue->consultLocalData()->isFile() ? tr("Unable to remove the local file") : tr("Unable to remove the local folder");
                ui->chooseRemoteCopy->setFailed(true, errorStr);
                mFailedItem = ui->chooseRemoteCopy;
                break;
            }
            case LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL:
            {
                auto errorStr = issue->consultLocalData()->isFile() ? tr("Unable to remove the file stored in MEGA")
                                                                    : tr("Unable to remove the folder stored in MEGA");
                ui->chooseLocalCopy->setFailed(true, errorStr);
                mFailedItem = ui->chooseLocalCopy;
                break;
            }
            case LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::BOTH:
            {
                auto errorStr = issue->consultLocalData()->isFile() ? tr("Unable to update both local and MEGA files")
                                                                    : tr("Unable to update both local and MEGA folders");
                ui->keepBothOption->setFailed(true, errorStr);
                mFailedItem = ui->keepBothOption;
                break;
            }
            default:
            {
                break;
            }
        }

    }

    updateSizeHint();
    ui->retranslateUi(this);
}

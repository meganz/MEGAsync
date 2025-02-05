#include "FolderMatchedAgainstFileWidget.h"

#include "FolderMatchedAgainstFileIssue.h"
#include "RenameNodeDialog.h"
#include "StalledIssueHeader.h"
#include "ui_FolderMatchedAgainstFileWidget.h"

#include <Preferences/Preferences.h>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QMessageBox>

FolderMatchedAgainstFileWidget::FolderMatchedAgainstFileWidget(QWidget* parent):
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::FolderMatchedAgainstFileWidget)
{
    ui->setupUi(this);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->chooseLayout->setContentsMargins(margins);
}

FolderMatchedAgainstFileWidget::~FolderMatchedAgainstFileWidget()
{
    delete ui;
}

void FolderMatchedAgainstFileWidget::refreshUi()
{
    const auto issue = getData().convert<FolderMatchedAgainstFileIssue>();
    const auto result(issue->getResult());

    if(issue->consultLocalData())
    {
        ui->localCopy->updateUi(issue->consultLocalData());
        ui->localCopy->setActionButtonVisibility(false);

        ui->localCopy->show();
    }
    else
    {
        ui->localCopy->hide();
    }

    if(issue->consultCloudData())
    {
        ui->remoteCopy->updateUi(issue->consultCloudData());
        ui->remoteCopy->setActionButtonVisibility(false);
        ui->remoteCopy->show();
    }
    else
    {
        ui->remoteCopy->hide();
    }

    if (issue->isSolved())
    {
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));

        switch(result.sideRenamed)
        {
            case StalledIssuesUtilities::KeepBothSidesState::REMOTE:
            {
                ui->remoteCopy->setMessage(QApplication::translate("NameConflict", "Renamed to \"%1\"").arg(result.newName), icon.pixmap(16,16), QString());
                break;
            }
            case StalledIssuesUtilities::KeepBothSidesState::LOCAL:
            {
                ui->localCopy->setMessage(QApplication::translate("NameConflict", "Renamed to \"%1\"").arg(result.newName), icon.pixmap(16,16), QString());
                break;
            }
            default:
            {
            }
        }
    }
    else if(issue->isFailed())
    {
        ui->remoteCopy->setFailed(true, RenameRemoteNodeDialog::renamedFailedErrorString(result.error.get(), issue->consultCloudData()->getNode()->isFile()));
        ui->localCopy->setFailed(true, RenameLocalNodeDialog::renamedFailedErrorString(issue->consultLocalData()->isFile()));
    }

    updateSizeHint();
    ui->retranslateUi(this);
}

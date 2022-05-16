#include "NameConflict.h"
#include "ui_NameConflict.h"

#include "StalledIssueHeader.h"
#include "StalledIssueChooseTitle.h"

#include <MegaApplication.h>
#include "RenameDialog.h"
#include "QMegaMessageBox.h"

#include <QDialogButtonBox>

static const int RENAME_ID = 0;
static const int REMOVE_ID = 1;

NameConflict::NameConflict(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameConflict)
{
    ui->setupUi(this);

    ui->path->setIndent(StalledIssueHeader::ICON_INDENT);
}

NameConflict::~NameConflict()
{
    delete ui;
}

void NameConflict::updateUi(NameConflictedStalledIssue::NameConflictData data)
{
    mData = data;
    ui->path->hide();

    if(mData.data)
    {
        ui->path->show();
        ui->path->updateUi(mData.data);
    }

    //Fill conflict names
    auto conflictedNames = mData.conflictedNames;
    auto nameConflictWidgets = ui->nameConflicts->findChildren<StalledIssueActionTitle*>();

    for(int index = 0; index < conflictedNames.size(); ++index)
    {
        if(nameConflictWidgets.size() > index)
        {
            nameConflictWidgets.at(index)->setTitle(conflictedNames.at(index));
        }
        else
        {
            auto chooseFile = new StalledIssueActionTitle(this);
            chooseFile->setIndent(StalledIssueHeader::ICON_INDENT);
            chooseFile->setTitle(conflictedNames.at(index));
            chooseFile->addActionButton(tr("Rename"), RENAME_ID);
            chooseFile->addActionButton(tr("Remove"), REMOVE_ID);

            connect(chooseFile, &StalledIssueActionTitle::actionClicked, this, &NameConflict::onActionClicked);

            ui->nameConflictsLayout->addWidget(chooseFile);
        }
    }
}

void NameConflict::onActionClicked(int actionId)
{
    if(auto chooseTitle = dynamic_cast<StalledIssueActionTitle*>(sender()))
    {
        QFileInfo info;
        info.setFile(mData.data->getNativePath(), chooseTitle->title());

        auto delegateWidget = dynamic_cast<StalledIssueBaseDelegateWidget*>(parentWidget());

        //As a dialog will be shown, avoid removing the editor while the dialog is open
        if(delegateWidget)
        {
            delegateWidget->setKeepEditor(true);
        }

        bool actionAccepted(true);

        if(actionId == RENAME_ID)
        {
            QPointer<NameConflict> currentWidget = QPointer<NameConflict>(this);

            RenameDialog dialog;
            dialog.init(mData.data->isCloud(), info.filePath());
            auto result = dialog.exec();

            if(result == QDialog::Rejected)
            {
                actionAccepted = false;
            }

            if(!currentWidget)
            {
                return;
            }
        }
        else if(actionId == REMOVE_ID)
        {
            QPointer<NameConflict> currentWidget = QPointer<NameConflict>(this);

            if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                                     tr("Are you sure you want to remote the %1 %2?").arg(info.isFile() ? tr("file") : tr("folder")).arg(info.fileName()),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                    != QMessageBox::Yes
                    || !currentWidget)
            {
                actionAccepted = false;
            }
            else
            {
                if(mData.isCloud)
                {
                    mUtilities.removeRemoteFile(info.filePath());
                }
                else
                {
                    mUtilities.removeLocalFile(QDir::toNativeSeparators(info.filePath()));
                }
            }
        }

        //Now, close the editor beacuse the action has been finished
        if(delegateWidget)
        {
            delegateWidget->setKeepEditor(false);
        }

        if(actionAccepted)
        {
            //This updates the stalled issues...but it is too fast
            emit actionFinished();
        }
    }
}

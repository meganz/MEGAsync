#include "RemoveBackupDialog.h"
#include "ui_RemoveBackupDialog.h"
#include "MegaApplication.h"
#include "node_selector/gui/NodeSelectorSpecializations.h"
#include "MegaNodeNames.h"
#include "Utilities.h"
#include "DialogOpener.h"

#include <QButtonGroup>

RemoveBackupDialog::RemoveBackupDialog(std::shared_ptr<SyncSettings> backup, QWidget *parent) :
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mUi(new Ui::RemoveBackupDialog),
    mBackup(backup),
    mTargetFolder(MegaSyncApp->getRootNode()->getHandle())
{
    mUi->setupUi(this);
    mUi->lTarget->setReadOnly(true);
    connect(mUi->bConfirm, &QPushButton::clicked, this, &QDialog::accept);
    connect(mUi->bCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(mUi->rMoveFolder, &QRadioButton::toggled, this, &RemoveBackupDialog::OnMoveSelected);
    connect(mUi->rDeleteFolder, &QRadioButton::toggled, this, &RemoveBackupDialog::OnDeleteSelected);
    connect(mUi->bChange, &QPushButton::clicked, this, &RemoveBackupDialog::OnChangeButtonClicked);

    mUi->bConfirm->setEnabled(false);
    mUi->moveToContainer->setEnabled(false);
    mUi->lTarget->setText(MegaNodeNames::getRootNodeName(MegaSyncApp->getRootNode().get())
                          .append(QLatin1Char('/')));
    adjustSize();
}

RemoveBackupDialog::~RemoveBackupDialog()
{
    delete mUi;
}

std::shared_ptr<SyncSettings> RemoveBackupDialog::backupToRemove()
{
    return mBackup;
}

//returns INVALID_HANDLE if user wants to delete the folder,
//otherwise return selected folder, cloud drive if nothing is selected
mega::MegaHandle RemoveBackupDialog::targetFolder()
{
    return mUi->rMoveFolder->isChecked() ? mTargetFolder : mega::INVALID_HANDLE;
}

void RemoveBackupDialog::OnDeleteSelected()
{
    mUi->bConfirm->setEnabled(true);
    mUi->moveToContainer->setEnabled(false);
}

void RemoveBackupDialog::OnMoveSelected()
{
    mUi->bConfirm->setEnabled(true);
    mUi->moveToContainer->setEnabled(true);
}

void RemoveBackupDialog::OnChangeButtonClicked()
{
    auto nodeSelector = new UploadNodeSelector(this);
    DialogOpener::showDialog<NodeSelector>(nodeSelector, [this, nodeSelector]
    {
        if (nodeSelector->result() == QDialog::Accepted)
        {
            mTargetFolder = nodeSelector->getSelectedNodeHandle();
            auto targetNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(mTargetFolder));
            auto targetRoot = std::unique_ptr<mega::MegaNode>(mMegaApi->getRootNode(targetNode.get()));

            mUi->lTarget->setText(MegaNodeNames::getRootNodeName(targetRoot.get())
                                  + QString::fromUtf8(mMegaApi->getNodePath(targetNode.get())));
        }
    });
}

#include "ChooseMoveBackupFolderErrorDialog.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorSpecializations.h"
#include "ui_ChooseMoveBackupFolderErrorDialog.h"
#include "Utilities.h"

#include <QButtonGroup>

ChooseMoveBackupFolderErrorDialog::ChooseMoveBackupFolderErrorDialog(
    mega::MegaHandle nonValidTargetFolder,
    QWidget* parent):
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mUi(new Ui::ChooseMoveBackupFolderErrorDialog),
    mTargetFolder(nonValidTargetFolder)
{
    mUi->setupUi(this);

    mUi->lTarget->setReadOnly(true);
    connect(mUi->bConfirm,
            &QPushButton::clicked,
            this,
            &ChooseMoveBackupFolderErrorDialog::onConfirmClicked);
    connect(mUi->bCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(mUi->bChange,
            &QPushButton::clicked,
            this,
            &ChooseMoveBackupFolderErrorDialog::onChangeButtonClicked);

    mUi->lTarget->setText(getFolderName(mTargetFolder));
}

ChooseMoveBackupFolderErrorDialog::~ChooseMoveBackupFolderErrorDialog()
{
    delete mUi;
}

void ChooseMoveBackupFolderErrorDialog::onConfirmClicked()
{
    emit moveBackup(mTargetFolder);
    close();
}

void ChooseMoveBackupFolderErrorDialog::onChangeButtonClicked()
{
    auto nodeSelector = new MoveBackupNodeSelector(this);
    nodeSelector->init();
    DialogOpener::showDialog<NodeSelector>(
        nodeSelector,
        [this, nodeSelector]
        {
            if (nodeSelector->result() == QDialog::Accepted)
            {
                if (mTargetFolder != nodeSelector->getSelectedNodeHandle())
                {
                    mTargetFolder = nodeSelector->getSelectedNodeHandle();
                    mUi->bConfirm->setEnabled(true);
                    mUi->lTarget->setText(getFolderName(mTargetFolder));
                }
            }
        });
}

QString ChooseMoveBackupFolderErrorDialog::getFolderName(mega::MegaHandle handle)
{
    auto targetNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(handle));
    auto targetRoot = std::unique_ptr<mega::MegaNode>(mMegaApi->getRootNode(targetNode.get()));

    return MegaNodeNames::getRootNodeName(targetRoot.get()) +
           QString::fromUtf8(mMegaApi->getNodePath(targetNode.get()));
}

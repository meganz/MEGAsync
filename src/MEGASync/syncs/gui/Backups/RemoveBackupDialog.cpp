#include "RemoveBackupDialog.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorSpecializations.h"
#include "StatsEventHandler.h"
#include "ui_RemoveBackupDialog.h"
#include "Utilities.h"

#include <QButtonGroup>

RemoveBackupDialog::RemoveBackupDialog(QWidget* parent):
    QDialog(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mUi(new Ui::RemoveBackupDialog),
    mTargetFolder(MegaSyncApp->getRootNode() ? MegaSyncApp->getRootNode()->getHandle() :
                                               mega::INVALID_HANDLE)
{
    mUi->setupUi(this);
    mUi->lTarget->setReadOnly(true);
    connect(mUi->bConfirm, &QPushButton::clicked, this, &RemoveBackupDialog::onConfirmClicked);
    connect(mUi->bCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(mUi->rMoveFolder, &QRadioButton::toggled, this, &RemoveBackupDialog::onMoveSelected);
    connect(mUi->rDeleteFolder,
            &QRadioButton::toggled,
            this,
            &RemoveBackupDialog::onDeleteSelected);
    connect(mUi->bChange, &QPushButton::clicked, this, &RemoveBackupDialog::onChangeButtonClicked);

    mUi->moveToContainer->setEnabled(false);
    mUi->lTarget->setText(MegaSyncApp->getRootNode() ?
                              MegaNodeNames::getRootNodeName(MegaSyncApp->getRootNode().get()) :
                              QString());
}

RemoveBackupDialog::~RemoveBackupDialog()
{
    delete mUi;
}

void RemoveBackupDialog::onConfirmClicked()
{
    auto targetFolder = mUi->rMoveFolder->isChecked() ? mTargetFolder : mega::INVALID_HANDLE;
    emit removeBackup(targetFolder);
    close();
}

void RemoveBackupDialog::onDeleteSelected()
{
    onOptionSelected(AppStatsEvents::EventType::DELETE_REMOVED_BAKCUP_CLICKED, false);
}

void RemoveBackupDialog::onMoveSelected()
{
    onOptionSelected(AppStatsEvents::EventType::MOVE_REMOVED_BACKUP_FOLDER, true);
}

void RemoveBackupDialog::onOptionSelected(const AppStatsEvents::EventType eventType,
                                          const bool enableMoveContainer)
{
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(eventType);
    mUi->moveToContainer->setEnabled(enableMoveContainer);
    mUi->bConfirm->setEnabled(true);
    mUi->rMoveFolder->setAutoExclusive(true);
    mUi->rDeleteFolder->setAutoExclusive(true);
}

void RemoveBackupDialog::onChangeButtonClicked()
{
    auto nodeSelector = new MoveBackupNodeSelector(this);
    nodeSelector->init();
    DialogOpener::showDialog<NodeSelector>(
        nodeSelector,
        [this, nodeSelector]
        {
            if (nodeSelector->result() == QDialog::Accepted)
            {
                mTargetFolder = nodeSelector->getSelectedNodeHandle();
                auto targetNode =
                    std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(mTargetFolder));
                auto targetRoot =
                    std::unique_ptr<mega::MegaNode>(mMegaApi->getRootNode(targetNode.get()));

                mUi->lTarget->setText(MegaNodeNames::getRootNodeName(targetRoot.get()) +
                                      QString::fromUtf8(mMegaApi->getNodePath(targetNode.get())));
            }
        });
}

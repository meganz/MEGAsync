#include "RemoveBackupDialog.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "NodeSelectorSpecializations.h"
#include "StatsEventHandler.h"
#include "TextDecorator.h"
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

    mUi->lTarget->setText(MegaSyncApp->getRootNode() ?
                              MegaNodeNames::getRootNodeName(MegaSyncApp->getRootNode().get()) :
                              QString());

    mUi->lErrorHint->setVisible(false);

    const QString label = QString::fromLatin1(" ") + tr("Stop backup");
    mUi->bConfirm->setText(label);

    Text::Bold boldDecorator;
    auto deleteText = tr("Folder will be deleted from MEGA. It won't be deleted from "
                         "your computer. [B]This action cannot be undone.[/B]");
    boldDecorator.process(deleteText);
    mUi->lDeleteFolder->setText(deleteText);

    auto description = tr(
        "To stop backing up this folder, you need to either [B]move it[/B] or [B]delete it[/B].");
    boldDecorator.process(description);
    mUi->lSecondaryText->setText(description);

    mUi->rMoveFolder->setAutoExclusive(true);
    mUi->rDeleteFolder->setAutoExclusive(true);
}

RemoveBackupDialog::~RemoveBackupDialog()
{
    delete mUi;
}

void RemoveBackupDialog::onConfirmClicked()
{
    auto targetFolder = mUi->rMoveFolder->isChecked() ? mTargetFolder : mega::INVALID_HANDLE;
    emit removeBackup(targetFolder);
}

void RemoveBackupDialog::setTargetFolderErrorHint(QString error)
{
    mUi->lTarget->setProperty("error", QLatin1String("true"));

    mUi->lErrorHint->setText(error);
    mUi->lErrorHint->setVisible(true);

    mUi->lTarget->style()->polish(mUi->lTarget);
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
    if (!enableMoveContainer)
    {
        clearTargetFolderError();
    }

    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(eventType);
    mUi->moveToContainer->setEnabled(enableMoveContainer);
}

void RemoveBackupDialog::clearTargetFolderError()
{
    mUi->lTarget->setProperty("error", QLatin1String("false"));
    mUi->lErrorHint->setVisible(false);
    mUi->lErrorHint->clear();
    mUi->lTarget->style()->polish(mUi->lTarget);
}

void RemoveBackupDialog::onChangeButtonClicked()
{
    clearTargetFolderError();

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

#include "SettingsQuickWidgetBase.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "Platform.h"
#include "QmlDialogWrapper.h"
#include "SyncController.h"
#include "SyncExclusions.h"
#include "SyncSettingsModelBase.h"
#include "Utilities.h"

#include <QApplication>
#include <QFileInfo>
#include <QMouseEvent>

SettingsQuickWidgetBase::SettingsQuickWidgetBase(SyncSettingsModelBase* model,
                                                 SyncController& controller,
                                                 QWidget* parent):
    MegaQuickWidget(parent),
    mModel(model),
    mController(controller)
{
    // The model is created by the subclass and passed in without a parent (it cannot
    // take `this` as a parent in the subclass initializer list, because at that point
    // this widget's QObject base is not yet constructed). Re-parent it here, where the
    // QObject base is alive, so it is destroyed together with the widget.
    mModel->setParent(this);

    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Expose the shared status enum (SyncSettingsModelBase::State) to QML under a
    // tab-neutral name so the shared SettingsListDelegate can reference it without
    // depending on either concrete model type.
    qmlRegisterUncreatableType<SyncSettingsModelBase>(
        "SettingsModel",
        1,
        0,
        "SettingsModel",
        QStringLiteral("SettingsModel is an abstract base exposed only for its enums"));
}

SyncSettingsModelBase* SettingsQuickWidgetBase::model() const
{
    return mModel;
}

void SettingsQuickWidgetBase::setContextMenuOpen(bool open)
{
    if (open)
        qApp->installEventFilter(this);
    else
        qApp->removeEventFilter(this);
}

bool SettingsQuickWidgetBase::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (!rect().contains(mapFromGlobal(mouseEvent->globalPos())))
        {
            emit closeContextMenu();
        }
    }
    return MegaQuickWidget::eventFilter(watched, event);
}

void SettingsQuickWidgetBase::exploreLocal(const QString& localFolder) const
{
    Platform::getInstance()->showInFolder(localFolder);
}

void SettingsQuickWidgetBase::openInMega(int index) const
{
    Utilities::openInMega(mModel->getSyncSetting(index)->getMegaHandle());
}

void SettingsQuickWidgetBase::pause(int index) const
{
    mController.setSyncToSuspend(mModel->getSyncSetting(index));
}

void SettingsQuickWidgetBase::resume(int index) const
{
    mController.setSyncToRun(mModel->getSyncSetting(index));
}

void SettingsQuickWidgetBase::openExclusionsDialog(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    QFileInfo syncDir(sync->getLocalFolder());
    if (syncDir.exists())
    {
        QPointer<QmlDialogWrapper<SyncExclusions>> exclusions =
            new QmlDialogWrapper<SyncExclusions>(this->parentWidget(), sync->getLocalFolder());

        DialogOpener::showDialog(exclusions);
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.parent = this->parentWidget();
        msgInfo.descriptionText = tr("Error opening megaignore file");
        MessageDialogOpener::warning(msgInfo);
    }
}

void SettingsQuickWidgetBase::rescan(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    MegaSyncApp->getMegaApi()->rescanSync(sync->backupId(), true);
}

void SettingsQuickWidgetBase::reboot(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    mController.resetSync(sync, mega::MegaSync::SyncRunningState::RUNSTATE_DISABLED);
}

void SettingsQuickWidgetBase::removeNonConfirmation(int index) const
{
    const auto& sync = mModel->getSyncSetting(index);
    mController.removeSync(sync);
}

void SettingsQuickWidgetBase::openOverQuotaDialog() const
{
    auto overQuotaDialog = MegaSyncApp->createOverquotaDialogIfNeeded();

    if (overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog);
    }
}

void SettingsQuickWidgetBase::sortModelByName(bool ascending)
{
    mModel->sortByName(ascending);
}

void SettingsQuickWidgetBase::sortModelByStatus(bool ascending)
{
    mModel->sortByStatus(ascending);
}

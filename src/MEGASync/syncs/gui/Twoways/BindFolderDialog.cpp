#include "BindFolderDialog.h"
#include "ui_BindFolderDialog.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

#include <QInputDialog>

using namespace mega;

BindFolderDialog::BindFolderDialog(MegaApplication* _app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog),
    mApp(_app)
{
    ui->setupUi(this);

    ui->bOK->setDefault(true);

    connect(ui->wBinder, &FolderBinder::selectionDone, this, &BindFolderDialog::allSelectionsDone);
    setFocusProxy(ui->bOK);
}

BindFolderDialog::~BindFolderDialog()
{
    delete ui;
}

MegaHandle BindFolderDialog::getMegaFolder()
{
    return ui->wBinder->selectedMegaFolder();
}

void BindFolderDialog::setMegaFolder(MegaHandle handle, bool disableUi)
{
    ui->wBinder->setSelectedMegaFolder(handle, disableUi);
}

QString BindFolderDialog::getLocalFolder()
{
    return ui->wBinder->selectedLocalFolder();
}

QString BindFolderDialog::getSyncName()
{
    return mSyncName;
}

void BindFolderDialog::on_bOK_clicked()
{
    QString localFolderPath = ui->wBinder->selectedLocalFolder();
    MegaApi *megaApi = mApp->getMegaApi();
    MegaHandle handle = ui->wBinder->selectedMegaFolder();

    std::shared_ptr<MegaNode> node {megaApi->getNodeByHandle(handle)};
    if (!localFolderPath.length() || !node)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.parent = this;
        msgInfo.text = tr("Please select a local folder and a MEGA folder");
        QMegaMessageBox::warning(msgInfo);
        return;
    }

    std::unique_ptr<const char []> cPath{megaApi->getNodePath(node.get())};
    mMegaPath = QString::fromUtf8(cPath.get());

    localFolderPath = QDir::toNativeSeparators(QDir(localFolderPath).canonicalPath());
    if (localFolderPath.isEmpty())
    {
        accept();
        return;
    }

    // Check that we can sync the selected local folder
    QString warningMessage;
    auto syncability (SyncController::isLocalFolderSyncable(localFolderPath, mega::MegaSync::TYPE_TWOWAY, warningMessage));

    // If OK, check that we can sync the selected remote folder
    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = std::max(SyncController::isRemoteFolderSyncable(node, warningMessage), syncability);
    }

    auto finishFunc = [this, localFolderPath](QPointer<QMessageBox> msg){
        if(!msg || msg->result() == QMessageBox::Yes)
        {
            mSyncName = SyncController::getSyncNameFromPath(localFolderPath);
            accept();
        }
    };

    if (syncability == SyncController::CAN_SYNC)
    {
        finishFunc(nullptr);
    }
    else
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.parent = this;
        msgInfo.text = warningMessage;

        if (syncability == SyncController::WARN_SYNC)
        {
            msgInfo.text += QLatin1Char('\n') + tr("Do you want to continue?");
            msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
            msgInfo.defaultButton = QMessageBox::No;
            msgInfo.finishFunc = finishFunc;
        }
        QMegaMessageBox::warning(msgInfo);
    }
}

void BindFolderDialog::allSelectionsDone()
{
    ui->bOK->setFocus();
}

bool BindFolderDialog::focusNextPrevChild(bool next)
{
#ifdef Q_OS_MACOS
    if(next && ui->bOK->hasFocus())
#else
    if(next && ui->bCancel->hasFocus())
#endif
    {
        ui->wBinder->setFocus();
        return true;
    }

    return QDialog::focusNextPrevChild(next);
}

void BindFolderDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

QString BindFolderDialog::getMegaPath() const
{
    return mMegaPath;
}

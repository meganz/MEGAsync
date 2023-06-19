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
}

BindFolderDialog::~BindFolderDialog()
{
    delete ui;
}

MegaHandle BindFolderDialog::getMegaFolder()
{
    return ui->wBinder->selectedMegaFolder();
}

void BindFolderDialog::setMegaFolder(MegaHandle handle)
{
    ui->wBinder->setSelectedMegaFolder(handle);
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
        QMegaMessageBox::warning(nullptr, QString(), tr("Please select a local folder and a MEGA folder"), QMessageBox::Ok);
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

    // Display warning if needed
    if (syncability == SyncController::CANT_SYNC)
    {
        QMegaMessageBox::warning(nullptr, QString(), warningMessage, QMessageBox::Ok);
        return;
    }
    else if (syncability == SyncController::WARN_SYNC
             && (QMegaMessageBox::warning(nullptr, QString(), warningMessage
                                         + QLatin1Char('\n')
                                         + tr("Do you want to continue?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
             == QMessageBox::No))
    {
        return;
    }

    mSyncName = SyncController::getSyncNameFromPath(localFolderPath);

    accept();
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

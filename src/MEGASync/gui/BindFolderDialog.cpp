#include "BindFolderDialog.h"
#include "ui_BindFolderDialog.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

#include <QInputDialog>

using namespace mega;

BindFolderDialog::BindFolderDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->app = app;
    ui->bOK->setDefault(true);
    highDpiResize.init(this);
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
    return syncName;
}

void BindFolderDialog::on_bOK_clicked()
{
    SyncController controller;
    QString localFolderPath = ui->wBinder->selectedLocalFolder();
    MegaApi *megaApi = app->getMegaApi();
    MegaHandle handle = ui->wBinder->selectedMegaFolder();

    std::unique_ptr<MegaNode> node {megaApi->getNodeByHandle(handle)};
    if (!localFolderPath.length() || !node)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("Please select a local folder and a MEGA folder"), QMessageBox::Ok);
        return;
    }

    std::unique_ptr<const char []> cPath{megaApi->getNodePath(node.get())};
    megaPath = QString::fromUtf8(cPath.get());

    localFolderPath = QDir::toNativeSeparators(QDir(localFolderPath).canonicalPath());
    if (localFolderPath.isEmpty())
    {
        accept();
        return;
    }

    // Check that we can sync the selected folder
    QString warningMessage;
    if(!controller.isLocalFolderSyncable(localFolderPath, MegaSync::TYPE_TWOWAY, warningMessage))
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), warningMessage, QMessageBox::Ok);
        return;
    }

    // Set the sync name
    syncName = SyncController::getSyncNameFromPath(localFolderPath);

    // We want the syncname to be unique, so prompt the user while it is not
    const auto syncNames (SyncModel::instance()->getSyncNames(MegaSync::TYPE_TWOWAY));
    while (syncNames.contains(syncName))
    {
        QPointer<QInputDialog> id = new QInputDialog(this);
        id->setWindowFlags(id->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        id->setWindowTitle(tr("Sync name"));
        id->setLabelText(tr("The name \"%1\" is already in use for another sync\n"
                            "Please enter a different name to identify this synced folder:").arg(syncName));
        id->setTextValue(syncName);
        int result = id->exec();

        if (!id || !result)
        {
            delete id;
            return;
        }

        QString text = id->textValue().trimmed();
        delete id;

        if (text.isEmpty())
        {
            return;
        }
        syncName = text;
    }

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
    return megaPath;
}

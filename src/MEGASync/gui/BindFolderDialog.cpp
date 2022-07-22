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
    mHighDpiResize.init(this);
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
    mMegaPath = QString::fromUtf8(cPath.get());

    localFolderPath = QDir::toNativeSeparators(QDir(localFolderPath).canonicalPath());
    if (localFolderPath.isEmpty())
    {
        accept();
        return;
    }

    // Check that we can sync the selected folder
    QString warningMessage;
    auto syncability (controller.isLocalFolderSyncable(localFolderPath, mega::MegaSync::TYPE_TWOWAY, warningMessage));

    if (syncability == SyncController::CANT_SYNC)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), warningMessage, QMessageBox::Ok);
        return;
    }
    else if (syncability == SyncController::WARN_SYNC
             && (QMegaMessageBox::warning(nullptr, tr("Warning"), warningMessage
                                         + QLatin1Char('/')
                                         + tr("Do you want to continue?"),
                                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
             == QMessageBox::No))
    {
        return;
    }

    // Set the sync name
    mSyncName = SyncController::getSyncNameFromPath(localFolderPath);

    // We want the syncname to be unique, so prompt the user while it is not
    const auto syncNames (SyncModel::instance()->getSyncNames(MegaSync::TYPE_TWOWAY));
    while (syncNames.contains(mSyncName))
    {
        QPointer<QInputDialog> id = new QInputDialog(this);
        id->setWindowFlags(id->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        id->setWindowTitle(tr("Sync name"));
        id->setLabelText(tr("The name \"%1\" is already in use for another sync\n"
                            "Please enter a different name to identify this synced folder:").arg(mSyncName));
        id->setTextValue(mSyncName);
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
        mSyncName = text;
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
    return mMegaPath;
}

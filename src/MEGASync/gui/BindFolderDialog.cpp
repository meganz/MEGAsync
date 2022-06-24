#include "BindFolderDialog.h"
#include "ui_BindFolderDialog.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"

#include <QInputDialog>

using namespace mega;

BindFolderDialog::BindFolderDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->app = app;
    SyncModel *model = SyncModel::instance();

    syncNames = model->getSyncNames(SyncModel::AllHandledSyncTypes);
    localFolders = model->getLocalFolders(SyncModel::AllHandledSyncTypes); //notice: this also takes into account !active ones
    megaFolderPaths = model->getMegaFolders(SyncModel::AllHandledSyncTypes);
    ui->bOK->setDefault(true);
    highDpiResize.init(this);
}

BindFolderDialog::BindFolderDialog(MegaApplication *app, QStringList syncNames,
                                   QStringList localFolders,
                                   QStringList megaFolderPaths,
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->app = app;
    this->syncNames = syncNames;
    this->localFolders = localFolders;
    this->megaFolderPaths = megaFolderPaths;
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

    std::unique_ptr<const char []> cPath{megaApi->getNodePath(node.get()) };
    if (cPath)
    {
        megaPath = QString::fromUtf8(cPath.get());
    }

    localFolderPath = QDir::toNativeSeparators(QDir(localFolderPath).canonicalPath());
    if (!localFolderPath.size())
    {
        accept();
        return;
    }

    QString warningMessage;
    if(controller.isFolderAlreadySynced(localFolderPath, MegaSync::TYPE_TWOWAY, warningMessage))
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), warningMessage, QMessageBox::Ok);
        return;
    }

    for (int i = 0; i < megaFolderPaths.size(); i++)
    {
        QString p = megaFolderPaths.at(i);

        if (megaPath.startsWith(p) && ((p.size() == megaPath.size()) || p.size() == 1 || megaPath[p.size()] == QChar::fromAscii('/')))
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("The selected MEGA folder is already synced"), QMessageBox::Ok);
            return;
        }
        else if (p.startsWith(megaPath) && ((p.size() == megaPath.size()) || megaPath.size() == 1 || p[megaPath.size()] == QChar::fromAscii('/')))
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("A synced folder cannot be inside another synced folder"), QMessageBox::Ok);
            return;
        }
    }

   bool repeated;
   syncName = QFileInfo(localFolderPath).fileName();
   if (syncName.isEmpty())
   {
       syncName = QDir::toNativeSeparators(localFolderPath);
   }
   syncName.remove(QDir::separator());

   do
   {
       repeated = false;
       for (int i = 0; i < syncNames.size(); i++)
       {
           if (!syncName.compare(syncNames[i]))
           {
                repeated = true;

                QPointer<QInputDialog> id = new QInputDialog(this);
                id->setWindowFlags(id->windowFlags() & ~Qt::WindowContextHelpButtonHint);
                id->setWindowTitle(tr("Sync name"));
                id->setLabelText(tr("The name \"%1\" is already in use for another sync\n"
                                    "Please enter a different name to identify this synced folder:").arg(syncName));
                int result = id->exec();

                if (!id || !result)
                {
                    delete id;
                    return;
                }

                QString text = id->textValue();
                text = text.trimmed();
                delete id;

                if (text.isEmpty())
                {
                    return;
                }
                syncName = text;
           }
       }
   } while (repeated);

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

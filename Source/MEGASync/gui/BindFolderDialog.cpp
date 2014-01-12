#include "BindFolderDialog.h"
#include "ui_BindFolderDialog.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include <QInputDialog>

BindFolderDialog::BindFolderDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    this->app = app;
    Preferences *preferences = Preferences::instance();
    syncNames = preferences->getSyncNames();
    localFolders = preferences->getLocalFolders();
    megaFolderHandles = preferences->getMegaFolderHandles();
}

BindFolderDialog::BindFolderDialog(MegaApplication *app, QStringList syncNames,
                                   QStringList localFolders,
                                   QList<long long> megaFolderHandles,
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    this->app = app;
    this->syncNames = syncNames;
    this->localFolders = localFolders;
    this->megaFolderHandles = megaFolderHandles;
}

BindFolderDialog::~BindFolderDialog()
{
    delete ui;
}

long long BindFolderDialog::getMegaFolder()
{
    return ui->wBinder->selectedMegaFolder();
}

QString BindFolderDialog::getLocalFolder()
{
    return ui->wBinder->selectedLocalFolder();
}

QString BindFolderDialog::getSyncName()
{
    return syncName;
}

void BindFolderDialog::on_buttonBox_accepted()
{
    QString localFolderPath = QDir::toNativeSeparators(QDir(ui->wBinder->selectedLocalFolder()).canonicalPath());
    MegaApi *megaApi = app->getMegaApi();
    long long handle = ui->wBinder->selectedMegaFolder();
    Node *node = megaApi->getNodeByHandle(handle);
    if(!localFolderPath.length() || !node)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select a local folder and a MEGA folder"), QMessageBox::Ok);
        return;
    }

    for(int i=0; i<localFolders.size(); i++)
    {
        QString c = QDir::toNativeSeparators(QDir(localFolders[i]).canonicalPath());
        if(localFolderPath.startsWith(c) && ((c.size() == localFolderPath.size()) || (localFolderPath[c.size()]==QChar::fromAscii('\\'))))
        {
            QMessageBox::warning(this, tr("Error"), tr("A synced folder cannot be inside another synced folder"), QMessageBox::Ok);
            return;
        }
        else if(c.startsWith(localFolderPath) && c[localFolderPath.size()]==QChar::fromAscii('\\'))
        {
            QMessageBox::warning(this, tr("Error"), tr("A synced folder cannot be inside another synced folder"), QMessageBox::Ok);
            return;
        }
    }

    for(int i=0; i<megaFolderHandles.size(); i++)
    {
        Node *n = megaApi->getNodeByHandle(megaFolderHandles[i]);
        if(n)
        {
            QString megaPath = QString::fromUtf8(megaApi->getNodePath(node));
            QString p = QString::fromUtf8(megaApi->getNodePath(n));
            if(megaPath.startsWith(p) && ((p.size() == megaPath.size()) || (megaPath[p.size()]==QChar::fromAscii('/'))))
            {
                QMessageBox::warning(this, tr("Error"), tr("The selected MEGA folder is already synced"), QMessageBox::Ok);
                return;
            }
            else if(megaPath==QString::fromAscii("/") || (p.startsWith(megaPath) && p[megaPath.size()]==QChar::fromAscii('/')))
            {
                if(handle == megaApi->getRootNode()->nodehandle)
                    QMessageBox::warning(this, tr("Error"), tr("Full account syncing is only possible without any selective syncs"), QMessageBox::Ok);
                else
                    QMessageBox::warning(this, tr("Error"), tr("The selected MEGA folder contains an already synced folder"), QMessageBox::Ok);
                return;
            }
        }
    }

   if(!Utilities::verifySyncedFolderLimits(localFolderPath))
   {
       QMessageBox::warning(this, tr("Warning"), tr("Local folder too large (this beta is limited to %1 folders or %2 files.\n"
            "Please, select another folder.").arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
            .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER), QMessageBox::Ok);
       return;
   }

   bool repeated;
   syncName = QFileInfo(localFolderPath).fileName();
   do {
       repeated = false;
       for(int i=0; i<syncNames.size(); i++)
       {
           if(!syncName.compare(syncNames[i]))
           {
                repeated = true;

                bool ok;
                QString text = QInputDialog::getText(this, tr("Sync name"),
                     tr("The name \"%1\" is already in use for another sync\n"
                     "Please enter a different name to identify this synced folder:").arg(syncName),
                     QLineEdit::Normal, syncName, &ok).trimmed();
                if (!ok && text.isEmpty())
                    return;

                syncName = text;
           }
       }
   }while(repeated);

   accept();
}

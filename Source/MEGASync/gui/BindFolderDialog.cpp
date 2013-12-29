#include "BindFolderDialog.h"
#include "ui_BindFolderDialog.h"
#include "MegaApplication.h"
#include "utils/Utils.h"
#include <QInputDialog>

BindFolderDialog::BindFolderDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BindFolderDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    this->app = app;
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
        QMessageBox::warning(this, tr("Error"), tr("Please, select one local folder and one MEGA folder"), QMessageBox::Ok);
        return;
    }

    Preferences *preferences = app->getPreferences();
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
    {
        QString c = QDir::toNativeSeparators(QDir(preferences->getLocalFolder(i)).canonicalPath());
        if(localFolderPath.startsWith(c) && ((c.size() == localFolderPath.size()) || (localFolderPath[c.size()]==QChar::fromAscii('\\'))))
        {
            QMessageBox::warning(this, tr("Error"), tr("The selected local folder is already synced"), QMessageBox::Ok);
            return;
        }
        else if(c.startsWith(localFolderPath) && c[localFolderPath.size()]==QChar::fromAscii('\\'))
        {
            QMessageBox::warning(this, tr("Error"), tr("The selected local folder contains an already synced folder"), QMessageBox::Ok);
            return;
        }

        Node *n = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
        if(n)
        {
            QString megaPath = QString::fromUtf8(megaApi->getNodePath(node));
            QString p = QString::fromUtf8(megaApi->getNodePath(n));
            if(megaPath.startsWith(p) && ((p.size() == megaPath.size()) || (megaPath[p.size()]==QChar::fromAscii('/'))))
            {
                QMessageBox::warning(this, tr("Error"), tr("The selected MEGA folder is already synced"), QMessageBox::Ok);
                return;
            }
            else if(p.startsWith(megaPath) && p[megaPath.size()]==QChar::fromAscii('/'))
            {
                QMessageBox::warning(this, tr("Error"), tr("The selected MEGA folder contains an already synced folder"), QMessageBox::Ok);
                return;
            }
        }
    }

   if(!Utils::verifySyncedFolderLimits(localFolderPath))
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
       for(int i=0; i<preferences->getNumSyncedFolders(); i++)
       {
           if(!syncName.compare(preferences->getSyncName(i)))
           {
                repeated = true;

                bool ok;
                QString text = QInputDialog::getText(this, tr("Sync name"),
                     tr("The name \"%1\" is already in use for another sync.\n"
                     "Please, enter another name to identify this synced folder:").arg(syncName),
                     QLineEdit::Normal, syncName, &ok).trimmed();
                if (!ok && text.isEmpty())
                    return;

                syncName = text;
           }
       }
   }while(repeated);

   accept();
}

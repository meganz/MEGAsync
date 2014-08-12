#include "FolderBinder.h"
#include "ui_FolderBinder.h"

#include "MegaApplication.h"

using namespace mega;

FolderBinder::FolderBinder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderBinder)
{
    selectedMegaFolderHandle = mega::UNDEF;
    ui->setupUi(this);
    app = (MegaApplication *)qApp;
    megaApi = app->getMegaApi();
}

FolderBinder::~FolderBinder()
{
    delete ui;
}

long long FolderBinder::selectedMegaFolder()
{
    return selectedMegaFolderHandle;
}

QString FolderBinder::selectedLocalFolder()
{
    return ui->eLocalFolder->text();
}

void FolderBinder::on_bLocalFolder_clicked()
{
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    if(!defaultPath.size())
    {
        #ifdef WIN32
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
            #else
                defaultPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
            #endif
        #else
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
            #else
                defaultPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
            #endif
        #endif
    }
    QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
                                                      defaultPath,
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    if(path.length())
    {
        QDir dir(path);
        if(!dir.exists() && !dir.mkpath(QString::fromAscii("."))) return;
        ui->eLocalFolder->setText(path);
    }
}

void FolderBinder::on_bMegaFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, true, this);
    nodeSelector->nodesReady();
    int result = nodeSelector->exec();
    if(result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *selectedFolder = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if(!selectedFolder)
    {
        selectedMegaFolderHandle = mega::UNDEF;
        delete nodeSelector;
        return;
    }

    const char *fPath = megaApi->getNodePath(selectedFolder);
    if(!fPath)
    {
        selectedMegaFolderHandle = mega::UNDEF;
        delete nodeSelector;
        delete selectedFolder;
        return;
    }

    ui->eMegaFolder->setText(QString::fromUtf8(fPath));
    delete nodeSelector;
    delete selectedFolder;
    delete fPath;
}

void FolderBinder::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

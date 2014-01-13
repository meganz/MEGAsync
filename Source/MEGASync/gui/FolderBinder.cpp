#include "FolderBinder.h"
#include "ui_FolderBinder.h"

#include "MegaApplication.h"

FolderBinder::FolderBinder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderBinder)
{
    selectedMegaFolderHandle = UNDEF;
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
    QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
                                                      ui->eLocalFolder->text(),
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
        return;

    selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    ui->eMegaFolder->setText(QString::fromUtf8(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle))));
}

void FolderBinder::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

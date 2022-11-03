#include "BackupRenameWidget.h"
#include "ui_BackupRenameWidget.h"

#include "Utilities.h"
#include "Platform.h"
#include "CommonMessages.h"

BackupRenameWidget::BackupRenameWidget(const QString& path, int number, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BackupRenameWidget),
    mPath(path)
{
    ui->setupUi(this);
    ui->hLine->setVisible(number > 1);
    ui->lError->hide();
    ui->lLocalFolder->setText(ui->lLocalFolder->text().arg(number));
    ui->lLocalFolderPath->setText(ui->lLocalFolderPath->text().arg(path));
    connect(ui->lLocalFolderPath, &QLabel::linkActivated,
            this, &BackupRenameWidget::openLocalPath);
}

BackupRenameWidget::~BackupRenameWidget()
{
    delete ui;
}

bool BackupRenameWidget::isNewNameValid(QStringList& backupNames)
{
    auto newName = getNewNameRaw();
    QString errText;

    if (Utilities::isNodeNameValid(newName))
    {
        backupNames.removeOne(newName);
        if (backupNames.contains(newName))
        {
            errText = tr("A folder named \"%1\" already exists in your Backups. Rename "
                         "the new folder to continue with the backup.").arg(newName);
        }
    }
    else
    {
        errText = CommonMessages::errorInvalidChars();
    }

    bool error (!errText.isEmpty());

    ui->lError->setText(errText);
    ui->lError->setVisible(error);

    return !error;
}

QString BackupRenameWidget::getNewNameRaw()
{
    return ui->leNewName->text().isEmpty() ? SyncController::getSyncNameFromPath(mPath) : ui->leNewName->text();;
}

QString BackupRenameWidget::getPath()
{
    return mPath;
}

void BackupRenameWidget::openLocalPath(QString link)
{
    Platform::showInFolder(link);
}

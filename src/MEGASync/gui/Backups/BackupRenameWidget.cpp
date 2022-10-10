#include "BackupRenameWidget.h"
#include "ui_BackupRenameWidget.h"

#include "Utilities.h"
#include "Platform.h"

BackupRenameWidget::BackupRenameWidget(const QString& path, int number, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BackupRenameWidget),
    mPath(path)
{
    ui->setupUi(this);
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

QString BackupRenameWidget::getNewName(QStringList brotherWdgNames)
{
    auto newName = getNewNameRaw();
    QString errText;

    if (!Utilities::isNodeNameValid(newName))
    {
        errText = tr("The following characters are not allowed:\n") + Utilities::FORBIDDEN_CHARS;
    }
    else
    {
        brotherWdgNames.removeOne(newName);
        if (brotherWdgNames.contains(newName))
        {
            errText = tr("A folder named \"%1\" already exists in your Backups. Rename "
                         "the new folder to continue with the backup.").arg(newName);
        }
    }

    if (!errText.isEmpty())
    {
        ui->lError->setText(errText);
        newName.clear();
    }
    ui->lError->setVisible(!errText.isEmpty());

    return newName;
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

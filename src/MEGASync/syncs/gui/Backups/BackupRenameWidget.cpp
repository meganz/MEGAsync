#include "BackupRenameWidget.h"
#include "ui_BackupRenameWidget.h"

#include "Utilities.h"
#include "Platform.h"
#include "CommonMessages.h"
#include "syncs/gui/SyncTooltipCreator.h"

BackupRenameWidget::BackupRenameWidget(const QString& path, int number, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BackupRenameWidget),
    mPath(path)
{
    ui->setupUi(this);
    ui->hLine->setVisible(number > 1);
    ui->lError->hide();
    ui->lLocalFolder->setText(ui->lLocalFolder->text().arg(number));
    ui->lLocalFolderPath->setToolTip(SyncTooltipCreator::createForLocal(mPath));
    ui->leNewName->installEventFilter(this);
    ui->lLocalFolderPath->installEventFilter(this);
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
    ui->lError->updateGeometry();

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

bool BackupRenameWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->leNewName && event->type() == QEvent::Resize)
    {
        auto elidedPath = ui->lLocalFolderPath->fontMetrics().elidedText(mPath,
                                                                         Qt::ElideMiddle,
                                                                         ui->leNewName->width());
        ui->lLocalFolderPath->setText(elidedPath);
    }
    else if(watched == ui->lLocalFolderPath && event->type() == QEvent::MouseButtonPress)
    {
        openLocalPath(mPath);
    }
    return QFrame::eventFilter(watched, event);
}

void BackupRenameWidget::openLocalPath(QString link)
{
    Platform::getInstance()->showInFolder(link);
}

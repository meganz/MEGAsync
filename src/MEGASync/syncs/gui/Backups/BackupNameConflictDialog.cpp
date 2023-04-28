#include "BackupNameConflictDialog.h"
#include "ui_BackupNameConflictDialog.h"
#include "syncs/gui/Backups/BackupRenameWidget.h"
#include "Utilities.h"
#include "syncs/control/SyncInfo.h"
#include "syncs/control/SyncController.h"
#include "EventHelper.h"
#include <QScrollBar>

#include <QPushButton>
#include <QtConcurrent/QtConcurrent>
#include <QDesktopServices>
#include <QSet>

BackupNameConflictDialog::BackupNameConflictDialog(const QStringList& candidatePaths, QWidget *parent):
    QDialog(parent),
    ui(new Ui::BackupNameConflictDialog)
{
    foreach(auto& path, candidatePaths)
    {
        QString syncNameFromPath = SyncController::getSyncNameFromPath(path); //TODO: SNC-3324
        mBackupNames.insert(path, syncNameFromPath.remove(Utilities::FORBIDDEN_CHARS_RX));
    }
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    ui->scrollArea->installEventFilter(this);
    ui->scrollArea->verticalScrollBar()->installEventFilter(this);

    ui->lToBackupCenterText->setVisible(false);
    ui->lToBackupCenterText->setTextFormat(Qt::RichText);
    QString toBackupCenterText = tr("If you don't want to rename the new folder, "
    "stop the backup in the [A]Backup centre[/A] for the existing folder. Then"
    " setup the backup for the new folder again.");

    toBackupCenterText.prepend(QString::fromUtf8("<html><head/><body><p>"));
    toBackupCenterText.append(QString::fromUtf8("</p></body></html>"));
    toBackupCenterText.replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"%1\"><span style=\" "
                                                                   "text-decoration: underline; color:#aa1a00;\">").arg(Utilities::BACKUP_CENTER_URL));
    toBackupCenterText.replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>"));

    ui->lToBackupCenterText->setText(toBackupCenterText);

    // Rename "Apply" button to "Rename and backup"
    auto bApply (ui->buttonBox->button(QDialogButtonBox::Apply));
    if (bApply)
    {
        bApply->setText(tr("Rename and backup"));
        bApply->setDefault(true);
        connect(bApply, &QPushButton::clicked,
                this, &BackupNameConflictDialog::checkChangedNames);
    }
    // Populate
    createWidgets();

    open();
}

BackupNameConflictDialog::~BackupNameConflictDialog()
{
    delete ui;
}

QMap<QString, QString> BackupNameConflictDialog::getChanges()
{
    return mBackupNames; //KEY:path VALUE:backup name
}

bool BackupNameConflictDialog::backupNamesValid(QStringList candidatePaths)
{
    QSet<QString> candidatesNames;
    bool areValid (true);
    auto pathIt (candidatePaths.cbegin());
    while (areValid && pathIt != candidatePaths.cend())
    {
        auto name = SyncController::getSyncNameFromPath(*pathIt);
        name.remove(Utilities::FORBIDDEN_CHARS_RX);
        //areValid = Utilities::isNodeNameValid(name); //TODO: SNC-3324
        if (areValid)
        {
            candidatesNames.insert(name);
            pathIt++;
        }
    }

    return areValid && candidatePaths.size() == candidatesNames.size()
            && !candidatesNames.intersects(SyncInfo::getRemoteBackupFolderNames());
}

bool BackupNameConflictDialog::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        if(obj == ui->scrollArea || obj == ui->scrollArea->verticalScrollBar())
        {
            auto verticalScrollArea = ui->scrollArea->verticalScrollBar()->isVisible() ? ui->scrollArea->verticalScrollBar()->width() : 0;
            ui->wConflictZone->setFixedWidth(ui->scrollArea->width() - verticalScrollArea);
        }
        else
        {
            updateScrollHeight();
        }
    }
    return QObject::eventFilter(obj, event);
}

void BackupNameConflictDialog::checkChangedNames()
{
    unsigned int failCount (0);
    const auto conflicts = ui->wConflictZone->findChildren<BackupRenameWidget*>();

    // Get the remote names
    QStringList chosenNames (SyncInfo::getRemoteBackupFolderNames().toList());

    // First pass to get all the new names:
    //   - First replace in candidate nams all the changed names
    foreach (auto conflict, conflicts)
    {
        mBackupNames.insert(conflict->getPath(),
                            conflict->getNewNameRaw());
    }
    //   - Then gather the updated backups names
    chosenNames << mBackupNames.values();

    // Second pass to check if we still have conflicts
    foreach (auto conflict, conflicts)
    {
        if(!conflict->isNewNameValid(chosenNames))
        {
            ++failCount;
        }
    }

    if (failCount == 0)
    {
        accept();
    }
}

void BackupNameConflictDialog::openLink(QString link)
{
    Utilities::openUrl(QUrl(link));
}

void BackupNameConflictDialog::createWidgets()
{
    QString conflictText;

    // Check conflicts and add widgets
    const auto currentNames = SyncInfo::getRemoteBackupFolderNames();
    for (auto it = mBackupNames.cbegin(); it != mBackupNames.cend(); it++)
    {
        if (currentNames.contains(it.value()))
        {   // Remote conflict
            addRenameWidget(it.key());
        }
        else
        {
            for (auto itIn = mBackupNames.cbegin(); itIn != mBackupNames.cend(); itIn++)
            {
                if (itIn != it && itIn.value() == it.value())
                {
                    // Local conflict
                    addRenameWidget(it.key());
                }
            }
        }
    }

    ui->scrollAreaLayout->addStretch();

    // Update texts
    auto conflicts = ui->wConflictZone->findChildren<BackupRenameWidget*>();
    int nbConflict = conflicts.size();
    if (nbConflict == 1)
    {
        QString name (mBackupNames[conflicts.first()->getPath()]);
        conflictText = tr("A folder named \"%1\" already exists in your Backups. Rename the new "
                          "folder to continue with the backup. "
                          "Folder name will not change on your computer.").arg(name);

        connect(ui->lToBackupCenterText, &QLabel::linkActivated,
                this, &BackupNameConflictDialog::openLink, Qt::UniqueConnection);

        ui->lToBackupCenterText->show();
        EventManager::addEvent(ui->scrollArea->viewport(), QEvent::Wheel, EventHelper::BLOCK);
    }
    else if(nbConflict > 1)
    {
        conflictText = tr("You can't back up folders with the same name. Rename them to "
                          "continue with the backup. Folder names won't change on your computer.");

        ui->lToBackupCenterText->hide();
    }

    updateScrollHeight();

#ifdef Q_OS_LINUX
    //It is the only OS where the bottom buttons will be over the conflict without this
    adjustSize();
    setMinimumHeight(sizeHint().height());
#endif

    ui->lConflictText->setText(conflictText);
    ui->lTitle->setText(tr("Rename folder", "", nbConflict));
}

void BackupNameConflictDialog::addRenameWidget(const QString& path)
{
    auto conflicts = ui->wConflictZone->findChildren<BackupRenameWidget*>();
    int conflictNumber = conflicts.size() + 1;
    foreach(auto& conflict, conflicts)
    {
        if(conflict->getPath() == path)
        {
            return;
        }
    }

    // Insert rename widget
    auto conflictItem = new BackupRenameWidget(path, conflictNumber);
    ui->scrollAreaLayout->addWidget(conflictItem,0, Qt::AlignTop);
    conflictItem->installEventFilter(this);
}

void BackupNameConflictDialog::updateScrollHeight()
{
    auto conflicts = ui->wConflictZone->findChildren<BackupRenameWidget*>();
    int nbConflict = conflicts.size();

    auto minHeight(0);
    for(int index = 0; index < std::min(nbConflict, 2); ++index)
    {
        auto conflict = conflicts.at(index);
        minHeight += conflict->sizeHint().height();
    }
    ui->scrollArea->setMinimumHeight(minHeight);
}

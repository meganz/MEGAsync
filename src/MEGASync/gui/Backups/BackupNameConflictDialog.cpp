#include "BackupNameConflictDialog.h"
#include "ui_BackupNameConflictDialog.h"
#include "Backups/BackupRenameWidget.h"
#include "Utilities.h"
#include "SyncController.h"
#include "gui/EventHelper.h"

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
        mBackupNames.insert(path, SyncController::getSyncNameFromPath(path));
    }
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    ui->lToBackupCenterText->setVisible(false);

    // Rename "Apply" button to "Rename and backup"
    auto buttonBox (findChild<QDialogButtonBox*>(QLatin1String("buttonBox")));
    if (buttonBox)
    {
        auto bApply (buttonBox->button(QDialogButtonBox::Apply));
        if (bApply)
        {
            bApply->setText(tr("Rename and backup"));
            connect(bApply, &QPushButton::clicked,
                    this, &BackupNameConflictDialog::checkChangedNames);
        }
    }

    // Populate
    ui->wConflictZone->setLayout(new QVBoxLayout());
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
        areValid = Utilities::isNodeNameValid(name);
        if (areValid)
        {
            candidatesNames.insert(name);
            pathIt++;
        }
    }

    return areValid && candidatePaths.size() == candidatesNames.size()
            && !candidatesNames.intersects(Utilities::getBackupsNames());;
}

void BackupNameConflictDialog::checkChangedNames()
{
    unsigned int failCount (0);
    const auto conflicts = ui->wConflictZone->findChildren<BackupRenameWidget*>();

    QStringList chosenNames;

    // First pass to get all the new names
    foreach (auto conflict, conflicts)
    {
        chosenNames << conflict->getNewNameRaw();
    }

    // Add the remote names
    chosenNames << Utilities::getBackupsNames().toList();

    // Second pass to check if we still have conflicts
    foreach (auto conflict, conflicts)
    {
        auto newName (conflict->getNewName(chosenNames));
        if(!newName.isEmpty())
        {
            mBackupNames.insert(conflict->getPath(), newName);
        }
        else
        {
            ++failCount;
        }
    }

    if (failCount == 0)
    {
        accept();
    }
    else
    {
        ui->scrollArea->setMinimumHeight(conflicts.at(0)->sizeHint().height() * std::min(2U, failCount));
    }
}

void BackupNameConflictDialog::openLink(QString link)
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(link));
}

void BackupNameConflictDialog::createWidgets()
{
    QString conflictText;

    // Check conflicts and add widgets
    const auto currentNames = Utilities::getBackupsNames();
    for (auto it = mBackupNames.cbegin(); it != mBackupNames.cend(); it++)
    {
        if (currentNames.contains(it.value()))
        {
            // Remote conflict
            addRenameWidget(it.key());
            continue;
        }

        for (auto itIn = mBackupNames.cbegin(); itIn != mBackupNames.cend(); itIn++)
        {
            if (itIn != it && itIn.value() == it.value())
            {
                // Local conflict
                addRenameWidget(it.key());
            }
        }
    }

    // Update texts
    auto conflicts = ui->wConflictZone->findChildren<BackupRenameWidget*>();
    int nbConflict = conflicts.size();
    if (nbConflict == 1)
    {
        QString name (mBackupNames[conflicts.first()->getPath()]);
        conflictText = tr("A folder named \"%1\" already exists in your Backups. Rename the new "
                          "folder to continue with the backup. "
                          "Folder name will not change on your computer. ").arg(name);

        connect(ui->lToBackupCenterText, &QLabel::linkActivated,
                this, &BackupNameConflictDialog::openLink, Qt::UniqueConnection);

        ui->lToBackupCenterText->show();
        ui->scrollArea->setMinimumHeight(conflicts.first()->height());
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        EventManager::addEvent(ui->scrollArea->viewport(), QEvent::Wheel, EventHelper::BLOCK);
    }
    else if(nbConflict > 1)
    {
        conflictText = tr("You can't back up folders with the same name."
                          " Rename them to continue with the backup."
                          " Folder name will not change on your computer.");

        ui->lToBackupCenterText->hide();
        ui->scrollArea->setMinimumHeight(conflicts.first()->height() * 2);
    }

    ui->lConflictText->setText(conflictText);
    ui->lTitle->setText(tr("Rename folder", "", nbConflict));
}

void BackupNameConflictDialog::insertHLine()
{
    auto hLine = new QFrame;
    hLine->setFrameShape(QFrame::HLine);
    hLine->setStyleSheet(QLatin1String("margin-right:25px; margin-left:25px;"
                                       "border: none;"
                                       "background: solid rgba(0, 0, 0, 0.0803);"
                                       "max-height: 1px;"));
    ui->wConflictZone->layout()->addWidget(hLine);
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

    // Insert line if needed (after the first conflict)
    if (conflictNumber > 1)
    {
        insertHLine();
    }
    // Insert rename widget
    ui->wConflictZone->layout()->addWidget(new BackupRenameWidget(path, conflictNumber));
}

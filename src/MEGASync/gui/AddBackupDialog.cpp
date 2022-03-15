#include "AddBackupDialog.h"
#include "ui_AddBackupDialog.h"

#include <QFileDialog>

AddBackupDialog::AddBackupDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AddBackupDialog),
    mSelectedFolder(QDir::home())
{
    mUi->setupUi(this);

#ifdef Q_OS_MACOS
    // Display our modal dialog embedded title label when parent is set
    mUi->embeddedTitleLabel->setVisible(this->parent() != nullptr);
#endif

    connect(mUi->addButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(mUi->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

AddBackupDialog::~AddBackupDialog()
{
    delete mUi;
}

void AddBackupDialog::setMyBackupsFolder(QString folder)
{
    mUi->backupToLabel->setText(folder);
}

QDir AddBackupDialog::getSelectedFolder()
{
    return mSelectedFolder;
}

void AddBackupDialog::on_changeButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choose Folder"),
                                                    QDir::home().path(),
                                                    QFileDialog::DontResolveSymlinks);
    if(folderPath.isEmpty())
        return;

    mSelectedFolder = folderPath;
    mUi->folderLineEdit->setText(mSelectedFolder.path());
    mUi->addButton->setEnabled(true);
}

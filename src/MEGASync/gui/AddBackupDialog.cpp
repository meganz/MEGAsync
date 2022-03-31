#include "AddBackupDialog.h"
#include "ui_AddBackupDialog.h"

#include <QFileDialog>

AddBackupDialog::AddBackupDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::AddBackupDialog),
    mSelectedFolder(QDir::home()),
    mMyBackupsFolder(),
    mSyncController(),
    mDeviceName()
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    mUi->setupUi(this);

    connect(&mSyncController, &SyncController::deviceName,
            this, &AddBackupDialog::onDeviceNameSet);

    mSyncController.getDeviceName();

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

void AddBackupDialog::setMyBackupsFolder(const QString& folder)
{
    mMyBackupsFolder = folder;
    QString fold = folder;
    if(!mDeviceName.isEmpty())
    {
        fold.append(QString::fromUtf8("/") + mDeviceName);
    }
    mUi->backupToLabel->setText(fold);
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

void AddBackupDialog::onDeviceNameSet(const QString &devName)
{
    mDeviceName = devName;
    if(!mUi->backupToLabel->text().isEmpty())
    {
        QString text = mUi->backupToLabel->text();
        text.append(QString::fromUtf8("/") + mDeviceName);
        mUi->backupToLabel->setText(text);
    }
}

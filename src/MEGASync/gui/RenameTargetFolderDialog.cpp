#include "RenameTargetFolderDialog.h"
#include "ui_RenameTargetFolderDialog.h"

RenameTargetFolderDialog::RenameTargetFolderDialog(const QString& syncName, FolderType type,
                                                   std::shared_ptr<mega::MegaNode> existingFolderNode,
                                                   QWidget* parent) :
    QDialog(parent),
    mUi(new Ui::RenameTargetFolderDialog),
    mSyncName (syncName)
{
    mUi->setupUi(this);

    // Rename "Save" button to "Rename"
    mUi->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Rename"));

    // Select appropriate page
    QWidget* page (nullptr);
    switch (type)
    {
        case TYPE_MYBACKUPS:
        {
            mUi->leNewFolderName->setPlaceholderText(tr("Rename your backup root folder"));
            page = mUi->pMyBackups;
            break;
        }
        case TYPE_BACKUP_FOLDER:
        default:
        {
            QString existingFolderUrl;
            if (existingFolderNode)
            {
                const char* handle = existingFolderNode->getBase64Handle();
                existingFolderUrl = QLatin1String("mega://#fm/") + QLatin1String(handle);
                delete [] handle;
            }
            else
            {
                existingFolderUrl = QLatin1String("https://mega.io");
            }

            mUi->lContentBackupFolder->setText(mUi->lContentBackupFolder->text()
                                               .arg(mSyncName, existingFolderUrl));
            mUi->leNewFolderName->setPlaceholderText(tr("Rename your new backup folder"));
            page = mUi->pBackupFolder;
            break;
        }
    }
    mUi->sContent->setCurrentWidget(page);

    // Disable "Save" button
    mUi->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    // Connect line edit to control
    connect(mUi->leNewFolderName, &QLineEdit::textEdited,
            this, &RenameTargetFolderDialog::onFolderNameFieldChanged);

    adjustSize();
}

RenameTargetFolderDialog::~RenameTargetFolderDialog()
{
    delete mUi;
}

QString RenameTargetFolderDialog::getNewFolderName() const
{
    return mUi->leNewFolderName->text();
}

void RenameTargetFolderDialog::onFolderNameFieldChanged(const QString& text)
{
    mUi->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!text.isEmpty());
}

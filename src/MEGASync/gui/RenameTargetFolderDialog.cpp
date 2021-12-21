#include "RenameTargetFolderDialog.h"
#include "ui_RenameTargetFolderDialog.h"
#include "MegaApplication.h"
#include "Utilities.h"

static const char* TEXT_TEMPLATE {"MEGA creates the top-level root folder "
                                  "for your backups. You already have a \"%1\" folder "
                                  "in your MEGA Cloud Storage. Please name your new root "
                                  "backup folder to something else."};

RenameTargetFolderDialog::RenameTargetFolderDialog(const QString& folderName,
                                                   QWidget* parent) :
    QDialog(parent),
    mUi(new Ui::RenameTargetFolderDialog)
{
    mUi->setupUi(this);

    // Rename "Save" button to "Rename"
    mUi->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Rename"));

    // Disable "Save" button
    mUi->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    // Set the text
    mOriginalText = mUi->lContentMyBackups->text();
    mUi->lContentMyBackups->setText(mOriginalText.arg(tr(TEXT_TEMPLATE).arg(folderName)));

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
    static mega::MegaApi* api (MegaSyncApp->getMegaApi());
    static std::unique_ptr<mega::MegaNode> nodeWithName;

    nodeWithName.reset(api->getNodeByPath(QString(QLatin1Char('/') + text).toUtf8().constData()));

    if (nodeWithName && !text.isEmpty())
    {
        mUi->lContentMyBackups->setText(mOriginalText.arg(tr(TEXT_TEMPLATE).arg(text)));
    }

    mUi->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!(nodeWithName || text.isEmpty()));
}

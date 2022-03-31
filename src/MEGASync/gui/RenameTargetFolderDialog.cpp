#include "RenameTargetFolderDialog.h"
#include "ui_RenameTargetFolderDialog.h"
#include "MegaApplication.h"
#include "Utilities.h"

RenameTargetFolderDialog::RenameTargetFolderDialog(QWidget* parent):
    QDialog(parent),
    mUi(new Ui::RenameTargetFolderDialog)
{
    mUi->setupUi(this);

    // Rename "Save" button to "Rename"
    mUi->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Rename"));

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
    static mega::MegaApi* api (MegaSyncApp->getMegaApi());
    static std::unique_ptr<mega::MegaNode> nodeWithName;
    auto vaultNode (MegaSyncApp->getVaultNode());
    auto vaultPath (QString::fromUtf8(api->getNodePath(vaultNode.get())));
    nodeWithName.reset(api->getNodeByPath(QString(vaultPath + QLatin1Char('/')
                                                  + text).toUtf8().constData()));
    mUi->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!(nodeWithName || text.isEmpty()));
}

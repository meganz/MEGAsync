#include "FileFolderNameSetterDialog.h"
#include "ui_FileFolderNameSetterDialog.h"
#include <Utilities.h>
#include <MegaApplication.h>
#include <mega/types.h>

#include <memory>

// Human-friendly list of forbidden chars for New Remote Folder
static const QString FORBIDDEN(QLatin1String("\\ / : \" * < > \? |"));
// Forbidden chars PCRE using a capture list: [\\/:"\*<>?|]
static const QRegularExpression FORBIDDEN_RX(QLatin1String("[\\\\/:\"*<>\?|]"));
// Time to show the new remote folder input error
static int NEW_FOLDER_ERROR_DISPLAY_TIME = 10000; //10s in milliseconds


FileFolderNameSetterDialog::FileFolderNameSetterDialog(std::shared_ptr<mega::MegaNode> parentNode, QWidget *parent)
    : QDialog(parent),
      mUi(new Ui::FileFolderNameSetterDialog()),
      mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
      mParentNode(parentNode)
{
}

int FileFolderNameSetterDialog::show()
{
    // Initialize the mNewFolder input Dialog
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    mUi->setupUi(this);

    // The dialog doesn't get resized on error
    mUi->textLabel->setMinimumSize(mUi->errorLabel->sizeHint());

    connect(mUi->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QPushButton *okButton = mUi->buttonBox->button(QDialogButtonBox::Ok);
    //only enabled when there's input, guards against empty folder name
    okButton->setEnabled(false);
    connect(mUi->lineEdit, &QLineEdit::textChanged, this, [this, okButton]()
    {
        bool hasText = !mUi->lineEdit->text().trimmed().isEmpty();
        okButton->setEnabled(hasText);
    });

    mNewFolderErrorTimer.setSingleShot(true);
    connect(&mNewFolderErrorTimer, &QTimer::timeout, this, &FileFolderNameSetterDialog::newFolderErrorTimedOut);
    connect(mUi->buttonBox, &QDialogButtonBox::accepted, this, &FileFolderNameSetterDialog::dialogAccepted);
    connect(mUi->buttonBox, &QDialogButtonBox::rejected, this, [this]
    {
        reject();
    });

    mUi->errorLabel->hide();
    mUi->textLabel->show();
    mUi->lineEdit->clear();
    mUi->lineEdit->setFocus();
    return exec();
}

QString FileFolderNameSetterDialog::getName() const
{
    return mUi->lineEdit->text().trimmed();
}

void FileFolderNameSetterDialog::showError()
{
    mUi->textLabel->hide();
    mUi->errorLabel->show();
    Utilities::animateFadein(mUi->errorLabel);
    mNewFolderErrorTimer.start(NEW_FOLDER_ERROR_DISPLAY_TIME); //(re)start timer
    mUi->lineEdit->setFocus();
}

void FileFolderNameSetterDialog::dialogAccepted()
{
    //TODO add it when we have an answer from content team
    //auto stillExists = MegaSyncApp->getMegaApi()->getNodeByHandle(mParentNode->getHandle());

    //        if(!stillExists || MegaSyncApp->getMegaApi()->isInRubbish(mParentNode.get()))
    //        {
    //            mUi->errorLabel->setText(tr("Error\nParent folder removed."));
    //            showError();
    //        }
    //        else
    {
        if(mUi->lineEdit->text().trimmed().contains(FORBIDDEN_RX))
        {
            mUi->errorLabel->setText(tr("The following characters are not allowed:\n%1").arg(FORBIDDEN));
            showError();
        }
        else
        {
            //dialog accepted, execute New Folder operation
            onDialogAccepted();
        }
    }
}

void FileFolderNameSetterDialog::newFolderErrorTimedOut()
{
    Utilities::animateFadeout(mUi->errorLabel);
    // after animation is finished, hide the error label and show the original text
    // 700 magic number is how long Utilities::  takes
    QTimer::singleShot(700, this, [this]()
    {
        mUi->errorLabel->hide();
        mUi->textLabel->show();
    });
}

///NEW FOLDER REIMPLMENETATION
NewFolderDialog::NewFolderDialog(std::shared_ptr<mega::MegaNode> parentNode, QWidget *parent)
    :FileFolderNameSetterDialog(parentNode, parent)
{
}

void NewFolderDialog::onDialogAccepted()
{
    QString newFolderName = getName();

    auto node = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByPath(newFolderName.toUtf8().constData(), mParentNode.get()));
    if (!node || node->isFile())
    {
        MegaSyncApp->getMegaApi()->createFolder(newFolderName.toUtf8().constData(), mParentNode.get(), mDelegateListener.get());
    }
    //Folder already exists
    else
    {
        mNewNode = std::move(node);
        done(QDialog::Rejected);
    }
}

void NewFolderDialog::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if(request->getType() == mega::MegaRequest::TYPE_CREATE_FOLDER)
    {
        int result(QDialog::Rejected);

        if (e->getErrorCode() == mega::MegaError::API_OK)
        {
            mNewNode = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(request->getNodeHandle()));
            if (mNewNode)
            {
                result = QDialog::Accepted;
            }
        }

        done(result);
    }
}

std::unique_ptr<mega::MegaNode> NewFolderDialog::getNewNode()
{
    return std::move(mNewNode);
}

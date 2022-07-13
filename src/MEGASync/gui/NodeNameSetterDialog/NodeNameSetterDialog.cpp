#include "NodeNameSetterDialog.h"
#include "ui_NodeNameSetterDialog.h"

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


NodeNameSetterDialog::NodeNameSetterDialog(QWidget *parent)
    : QDialog(parent),
      mUi(new Ui::NodeNameSetterDialog()),
      mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{
}

int NodeNameSetterDialog::show()
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

    mUi->lineEdit->setText(lineEditText());
    mUi->lineEdit->setSelection(lineEditSelection().start, lineEditSelection().length);

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, [this, okButton]()
    {
        bool hasText = !mUi->lineEdit->text().trimmed().isEmpty();
        okButton->setEnabled(hasText);
    });

    mUi->textLabel->setText(dialogText());

    mNewFolderErrorTimer.setSingleShot(true);
    connect(&mNewFolderErrorTimer, &QTimer::timeout, this, &NodeNameSetterDialog::newFolderErrorTimedOut);
    connect(mUi->buttonBox, &QDialogButtonBox::accepted, this, &NodeNameSetterDialog::dialogAccepted);
    connect(mUi->buttonBox, &QDialogButtonBox::rejected, this, [this]
    {
        reject();
    });

    mUi->errorLabel->hide();
    mUi->textLabel->show();
    mUi->lineEdit->setFocus();
    return exec();
}

QString NodeNameSetterDialog::getName() const
{
    return mUi->lineEdit->text().trimmed();
}

void NodeNameSetterDialog::showError(const QString &errorText)
{
    mUi->errorLabel->setText(errorText);

    mUi->textLabel->hide();
    mUi->errorLabel->show();
    Utilities::animateFadein(mUi->errorLabel);
    mNewFolderErrorTimer.start(NEW_FOLDER_ERROR_DISPLAY_TIME); //(re)start timer
    mUi->lineEdit->setFocus();
}

void NodeNameSetterDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
         mUi->textLabel->setText(dialogText());
    }

    QDialog::changeEvent(event);
}

void NodeNameSetterDialog::dialogAccepted()
{
    //TODO add it when we have an answer from content team
    //auto stillExists = MegaSyncApp->getMegaApi()->getNodeByHandle(mParentNode->getHandle());

    //        if(!stillExists || MegaSyncApp->getMegaApi()->isInRubbish(mParentNode.get()))
    //        {
    //            showError(tr("Error\nParent folder removed."));
    //        }
    //        else
    {
        if(mUi->lineEdit->text().trimmed().contains(FORBIDDEN_RX))
        {
            showError(tr("The following characters are not allowed:\n%1").arg(FORBIDDEN));
        }
        else
        {
            //dialog accepted, execute New Folder operation
            onDialogAccepted();
        }
    }
}

void NodeNameSetterDialog::newFolderErrorTimedOut()
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

#include "NodeNameSetterDialog.h"

#include "CommonMessages.h"
#include "MegaApplication.h"
#include "ui_NodeNameSetterDialog.h"
#include "Utilities.h"

#include <memory>

NodeNameSetterDialog::NodeNameSetterDialog(QWidget *parent)
    : QDialog(parent),
      mUi(new Ui::NodeNameSetterDialog()),
      mDelegateListener(std::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{
}

NodeNameSetterDialog::~NodeNameSetterDialog()
{
    delete mUi;
}

void NodeNameSetterDialog::init()
{
    // Initialize the mNewFolder input Dialog
    mUi->setupUi(this);

    title();

    // The dialog doesn't get resized on error
    mUi->textLabel->setMinimumSize(mUi->errorLabel->sizeHint());

    //only enabled when there's input, guards against empty folder name
    mUi->okButton->setEnabled(false);

    mOriginalName = lineEditText();
    mUi->lineEdit->setText(mOriginalName);
    mUi->lineEdit->setSelection(lineEditSelection().start, lineEditSelection().length);

    connect(mUi->lineEdit,
            &QLineEdit::textChanged,
            this,
            [this]()
            {
                bool hasText = !mUi->lineEdit->text().trimmed().isEmpty();
                mUi->okButton->setEnabled(hasText);
            });

    mUi->textLabel->setText(dialogText());

    mNewFolderErrorTimer.setSingleShot(true);
    connect(&mNewFolderErrorTimer, &QTimer::timeout, this, &NodeNameSetterDialog::newFolderErrorTimedOut);
    connect(mUi->okButton, &QPushButton::clicked, this, &NodeNameSetterDialog::dialogAccepted);
    connect(mUi->cancelButton,
            &QPushButton::clicked,
            this,
            [this]
            {
                reject();
            });

    mUi->errorContainer->hide();
    mUi->lineEdit->setFocus();
}

QString NodeNameSetterDialog::getName() const
{
    return mUi->lineEdit->text().trimmed();
}

QString NodeNameSetterDialog::getOriginalName() const
{
    return mOriginalName;
}

void NodeNameSetterDialog::showError(const QString &errorText)
{
    mUi->errorLabel->setText(errorText);

    mUi->errorContainer->show();
    Utilities::animateFadein(mUi->errorContainer);
    mNewFolderErrorTimer.start(Utilities::ERROR_DISPLAY_TIME_MS); //(re)start timer
    mUi->lineEdit->setFocus();
    mUi->lineEdit->setProperty("error", true);
    setStyleSheet(styleSheet());
}

bool NodeNameSetterDialog::checkAlreadyExistingNode(const QString& nodeName, std::shared_ptr<mega::MegaNode> parentNode)
{
    std::unique_ptr<mega::MegaNodeList>nodes(MegaSyncApp->getMegaApi()->getChildren(parentNode.get()));
    for(int index = 0; index < nodes->size(); ++index)
    {
        QString remoteNodeName(QString::fromUtf8(nodes->get(index)->getName()));
        if (nodeName.compare(remoteNodeName, Qt::CaseSensitive) == 0)
        {
            showAlreadyExistingNodeError(nodes->get(index)->isFile());
            return true;
        }
    }

    return false;
}

void NodeNameSetterDialog::showAlreadyExistingNodeError(bool isFile)
{
    isFile ? showError(tr("A file with this name already exists in this location.\nEnter a different name."))
           : showError(tr("A folder with this name already exists in this location.\nEnter a different name"));
}

bool NodeNameSetterDialog::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
         mUi->textLabel->setText(dialogText());
    }

    return QDialog::event(event);
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
        if(Utilities::isNodeNameValid(mUi->lineEdit->text()))
        {
            //dialog accepted, execute New Folder operation
            onDialogAccepted();
        }
        else
        {
            showError(CommonMessages::errorInvalidChars());
        }
    }
}

void NodeNameSetterDialog::newFolderErrorTimedOut()
{
    Utilities::animateFadeout(mUi->errorContainer);
    // after animation is finished, hide the error label and show the original text
    // 700 magic number is how long Utilities::  takes
    QTimer::singleShot(700,
                       this,
                       [this]()
                       {
                           mUi->errorContainer->hide();
                           mUi->lineEdit->setProperty("error", false);
                           setStyleSheet(styleSheet());
                       });
}

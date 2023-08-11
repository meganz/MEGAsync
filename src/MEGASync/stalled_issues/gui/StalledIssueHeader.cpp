#include "StalledIssueHeader.h"

#include <stalled_issues/model/StalledIssuesModel.h>
#include <stalled_issues/gui/stalled_issues_cases/StalledIssuesCaseHeaders.h>

#include <MegaApplication.h>
#include <Preferences.h>

#include "Utilities.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <QMegaMessageBox.h>

#include <QFile>
#include <QMouseEvent>

const int StalledIssueHeader::ARROW_INDENT = 6 + 16; //Left margin + arrow;
const int StalledIssueHeader::ICON_INDENT = 8 + 48; // fileIcon + spacer;
const int StalledIssueHeader::BODY_INDENT = StalledIssueHeader::ARROW_INDENT + StalledIssueHeader::ICON_INDENT; // full indent;
const int StalledIssueHeader::GROUPBOX_INDENT = BODY_INDENT - 9;// Following the InVision mockups
const int StalledIssueHeader::GROUPBOX_CONTENTS_INDENT = 9;// Following the InVision mockups
const int StalledIssueHeader::HEIGHT = 60;

const char* FILENAME_PROPERTY = "FILENAME_PROPERTY";
const char* MULTIPLE_ACTIONS_PROPERTY = "ACTIONS_PROPERTY";

StalledIssueHeader::StalledIssueHeader(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader),
    mIsExpandable(true)
{
    ui->setupUi(this);
    connect(ui->multipleActionButton, &QPushButton::clicked, this, &StalledIssueHeader::onMultipleActionClicked);
}

StalledIssueHeader::~StalledIssueHeader()
{
    delete ui;
}

void StalledIssueHeader::expand(bool state)
{
    auto arrowIcon = Utilities::getCachedPixmap(state ? QLatin1Literal(":/images/node_selector/Icon-Small-Arrow-Down.png") :  QLatin1Literal(":/images/node_selector/Icon-Small-Arrow-Left.png"));
    ui->arrow->setPixmap(arrowIcon.pixmap(ui->arrow->size()));
}

bool StalledIssueHeader::isExpandable() const
{
    return mIsExpandable;
}

void StalledIssueHeader::setIsExpandable(bool newIsExpandable)
{
    mIsExpandable = newIsExpandable;
    ui->arrow->setVisible(mIsExpandable);
}

bool StalledIssueHeader::adaptativeHeight()
{
    return false;
}

void StalledIssueHeader::on_ignoreFileButton_clicked()
{
    propagateButtonClick();

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    auto isSymLink(getData().consultData()->isSymLink());
    auto canBeIgnoredChecker = [isSymLink](const std::shared_ptr<const StalledIssue> issue){
        return isSymLink == issue->isSymLink() && issue->canBeIgnored();
    };

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));

    auto selection = dialog->getDialog()->getSelection(canBeIgnoredChecker);

    if(selection.size() <= 1)
    {
        auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(canBeIgnoredChecker);

        if(allSimilarIssues.size() != selection.size())
        {
            msgInfo.buttons |= QMessageBox::Yes;
            textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
            textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Ok"));
        }

    }
    else
    {
        textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
    }

    msgInfo.buttonsText = textsByButton;
    msgInfo.text = tr("Are you sure you want to ignore this issue?");
    msgInfo.informativeText = tr("This action will ignore this issue and it will not be synced.");

    msgInfo.finishFunc = [this, selection](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            MegaSyncApp->getStalledIssuesModel()->ignoreItems(selection);
        }
        else if(msgBox->result() == QDialogButtonBox::Yes)
        {
            MegaSyncApp->getStalledIssuesModel()->ignoreItems(QModelIndexList());
        }
    };

    QMegaMessageBox::warning(msgInfo);
}


void StalledIssueHeader::hideIgnoreFile()
{
    ui->ignoreFileButton->hide();
}

void StalledIssueHeader::showIgnoreFile()
{
    ui->ignoreFileButton->show();
}

void StalledIssueHeader::issueIgnored()
{
    ui->ignoreFileButton->hide();
    QIcon icon(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
    showSolvedMessage(tr("Ignored"));
}

void StalledIssueHeader::propagateButtonClick()
{
    QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::AltModifier));
    qApp->processEvents();
}

void StalledIssueHeader::showAction(const QString &actionButtonText)
{
    ui->actionContainer->show();
    ui->actionButton->setVisible(true);
    ui->actionButton->setText(actionButtonText);
}

void StalledIssueHeader::hideAction()
{
    ui->actionButton->setVisible(false);
}

void StalledIssueHeader::updateHeaderSizes()
{
    layout()->activate();

    ui->actionContainer->layout()->activate();
    ui->actionContainer->updateGeometry();

    ui->titleContainer->layout()->activate();
    ui->titleContainer->updateGeometry();

    ui->errorContainer->layout()->activate();
    ui->errorContainer->updateGeometry();

    ui->errorDescription->layout()->activate();
    ui->errorDescription->updateGeometry();

    ui->errorTitle->layout()->activate();
    ui->errorTitle->updateGeometry();

    ui->errorTitleTextContainer->layout()->activate();
    ui->errorTitleTextContainer->updateGeometry();

    ui->fileNameTitle->updateGeometry();
    ui->errorDescriptionText->updateGeometry();

}

void StalledIssueHeader::showMultipleAction(const QString &actionButtonText, const QList<ActionInfo>& actions)
{
    ui->actionContainer->show();
    ui->multipleActionButton->setVisible(true);
    ui->multipleActionButton->setText(actionButtonText);
    ui->multipleActionButton->setProperty(MULTIPLE_ACTIONS_PROPERTY, QVariant::fromValue<QList<ActionInfo>>(actions));
}

void StalledIssueHeader::hideMultipleAction()
{
    ui->multipleActionButton->setVisible(false);
}

void StalledIssueHeader::onMultipleActionClicked()
{
    propagateButtonClick();

    auto actions(ui->multipleActionButton->property(MULTIPLE_ACTIONS_PROPERTY).value<QList<ActionInfo>>());
    if(!actions.isEmpty())
    {
        QMenu *menu(new QMenu(ui->multipleActionButton));
        Platform::getInstance()->initMenu(menu, "MultipleActionStalledIssues");
        menu->setAttribute(Qt::WA_DeleteOnClose);

        foreach(auto action, actions)
        {
            // Show in system file explorer action
            auto actionItem (new MenuItemAction(action.actionText, QString()));
            auto id(action.id);
            connect(actionItem, &MenuItemAction::triggered, this, [this, id]()
            {
                mHeaderCase->onMultipleActionButtonOptionSelected(this, id);
            });
            actionItem->setParent(menu);
            menu->addAction(actionItem);
        }

        auto pos(ui->actionContainer->mapToGlobal(ui->multipleActionButton->pos()));
        pos.setY(pos.y() + ui->multipleActionButton->height());
        menu->popup(pos);
    }
}

void StalledIssueHeader::showMessage(const QString &message, const QPixmap& pixmap)
{
    ui->actionContainer->show();
    ui->actionMessageContainer->setVisible(true);
    ui->actionMessage->setText(message);

    if(!pixmap.isNull())
    {
        ui->actionMessageIcon->setPixmap(pixmap);
    }
}

void StalledIssueHeader::showSolvedMessage(const QString& customMessage)
{
    QIcon icon(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
    showMessage(customMessage.isEmpty() ? tr("Solved") : customMessage, icon.pixmap(24,24));

    if(!ui->titleContainer->graphicsEffect())
    {
        auto fileNameEffect = new QGraphicsOpacityEffect(this);
        fileNameEffect->setOpacity(0.30);
        ui->titleContainer->setGraphicsEffect(fileNameEffect);
    }
}

void StalledIssueHeader::setText(const QString &text)
{
    if(text.isEmpty())
    {
        ui->fileNameTitle->hide();
    }
    else
    {
        ui->fileNameTitle->show();
        ui->fileNameTitle->setText(text);
    }
}

QString StalledIssueHeader::displayFileName(bool preferCloud)
{
    return getData().consultData()->getFileName(preferCloud);
}

void StalledIssueHeader::setTitleDescriptionText(const QString &text)
{
    if(text.isEmpty())
    {
        ui->errorDescription->hide();
    }
    else
    {
        ui->errorDescription->show();
        ui->errorDescriptionText->setText(text);
    }
}

void StalledIssueHeader::setData(StalledIssueHeaderCase * issueData)
{
    mHeaderCase = issueData;
}

void StalledIssueHeader::reset()
{
    StalledIssueBaseDelegateWidget::reset();
    mHeaderCase->deleteLater();
}

void StalledIssueHeader::refreshCaseTitles()
{
    if(mHeaderCase)
    {
        mHeaderCase->refreshCaseTitles(this);
    }
}

void StalledIssueHeader::refreshCaseActions()
{
    if(mHeaderCase)
    {
        mHeaderCase->refreshCaseActions(this);
    }
}

QString StalledIssueHeader::fileName()
{
    return QString();
}

void StalledIssueHeader::on_actionButton_clicked()
{
    if(mHeaderCase)
    {
        propagateButtonClick();

        mHeaderCase->onActionButtonClicked(this);
    }
}

void StalledIssueHeader::refreshUi()
{
    auto errorTitleIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/ico_menu_full.png"));
    ui->errorTitleIcon->setPixmap(errorTitleIcon.pixmap(ui->errorTitleIcon->size()));

    QIcon fileTypeIcon;
    QFileInfo fileInfo;

    //Get full path -> it can be taken from the cloud data or the local data.
    if(getData().consultData()->consultLocalData())
    {
        fileInfo.setFile(getData().consultData()->consultLocalData()->getNativeFilePath());
    }
    else if(getData().consultData()->consultCloudData())
    {
        fileInfo.setFile(getData().consultData()->consultCloudData()->getNativeFilePath());
    }

    if(getData().consultData()->hasFiles() > 0)
    {
        fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                      getData().consultData()->getFileName(false), QLatin1Literal(":/images/drag_")));
    }
    else
    {
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/folder_orange_default@2x.png"));
    }

    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));

    resetSolvingWidgets();

    if(getData().consultData()->canBeIgnored())
    {
        getData().consultData()->isSolved() ? issueIgnored() : showIgnoreFile();
    }
    else
    {
        ui->ignoreFileButton->hide();

        if(getData().consultData()->isSolved())
        {
            showSolvedMessage();
        }
    }

   //By default it is expandable
   setIsExpandable(true);
}

void StalledIssueHeader::resetSolvingWidgets()
{
    ui->actionButton->hide();
    ui->multipleActionButton->hide();
    ui->actionMessageContainer->hide();
}

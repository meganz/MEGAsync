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

StalledIssueHeader::StalledIssueHeader(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader)
{
    ui->setupUi(this);

    ui->actionButton->hide();
    ui->multipleActionButton->hide();
    ui->actionMessageContainer->hide();
    ui->ignoreFileButton->hide();

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

bool StalledIssueHeader::adaptativeHeight()
{
    return false;
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

void StalledIssueHeader::clearLabels()
{
    ui->fileNameTitle->clear();

    ui->errorTitleTextContainer->removeEventFilter(this);
}

void StalledIssueHeader::propagateButtonClick()
{
    QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::NoModifier));
    qApp->processEvents();

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    dialog->getDialog()->updateView();
}

void StalledIssueHeader::showAction(const QString &actionButtonText)
{
    ui->actionButton->setVisible(true);
    ui->actionButton->setText(actionButtonText);
}

void StalledIssueHeader::hideAction()
{
    ui->actionButton->setVisible(false);
}

void StalledIssueHeader::showMultipleAction(const QString &actionButtonText, const QStringList &actions)
{
    ui->multipleActionButton->setVisible(true);
    ui->multipleActionButton->setText(actionButtonText);
    ui->multipleActionButton->setProperty("ACTIONS", actions);
}

void StalledIssueHeader::hideMultipleAction()
{
    ui->multipleActionButton->setVisible(false);
}

void StalledIssueHeader::onMultipleActionClicked()
{
    propagateButtonClick();

    auto actions(ui->multipleActionButton->property("ACTIONS").toStringList());
    if(!actions.isEmpty())
    {
        QMenu *menu(new QMenu(ui->multipleActionButton));
        Platform::getInstance()->initMenu(menu, "MultipleActionStalledIssues");
        menu->setAttribute(Qt::WA_DeleteOnClose);

        int index(0);
        foreach(auto action, actions)
        {
            // Show in system file explorer action
            auto actionItem (new MenuItemAction(action, QString()));
            connect(actionItem, &MenuItemAction::triggered, this, [this, index]()
            {
                mHeaderCase->onMultipleActionButtonOptionSelected(this, index);
            });
            actionItem->setParent(menu);
            menu->addAction(actionItem);

            index++;
        }

        auto pos(ui->actionContainer->mapToGlobal(ui->multipleActionButton->pos()));
        pos.setY(pos.y() + ui->multipleActionButton->height());
        menu->popup(pos);
    }
}

void StalledIssueHeader::showMessage(const QString &message, const QPixmap& pixmap)
{
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
    ui->fileNameTitle->setText(text);
}

QString StalledIssueHeader::displayFileName(bool preferCloud)
{
    return getData().consultData()->getFileName(preferCloud);
}

void StalledIssueHeader::setTitleDescriptionText(const QString &text)
{
    ui->errorDescriptionText->setText(text);
}

void StalledIssueHeader::setData(StalledIssueHeaderCase *data)
{
    mHeaderCase = data;
}

void StalledIssueHeader::reset()
{
    StalledIssueBaseDelegateWidget::reset();
    mHeaderCase->deleteLater();
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

void StalledIssueHeader::on_ignoreFileButton_clicked()
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    propagateButtonClick();

    auto canBeIgnoredChecker = [](const std::shared_ptr<const StalledIssue> issue){
        return issue->canBeIgnored();
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

void StalledIssueHeader::refreshUi()
{
    auto errorTitleIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/ico_menu_full.png"));
    ui->errorTitleIcon->setPixmap(errorTitleIcon.pixmap(ui->errorTitleIcon->size()));

    QIcon fileTypeIcon;
    QFileInfo fileInfo;

    clearLabels();

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

    if(getData().consultData()->canBeIgnored())
    {
        getData().consultData()->isSolved() ? issueIgnored() : showIgnoreFile();
    }
    else
    {
        ui->ignoreFileButton->hide();
    }

    if(mHeaderCase)
    {
        mHeaderCase->refreshCaseUi(this);
    }

    update();
}

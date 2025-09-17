#include "StalledIssueHeader.h"

#include "DialogOpener.h"
#include "IconTokenizer.h"
#include "IgnoredStalledIssue.h"
#include "MegaApplication.h"
#include "MessageDialogOpener.h"
#include "StalledIssuesCaseHeaders.h"
#include "StalledIssuesDialog.h"
#include "StalledIssuesModel.h"
#include "ThemeManager.h"
#include "TokenizableItems/TokenPropertyNames.h"
#include "TokenParserWidgetManager.h"
#include "Utilities.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QMouseEvent>

const int StalledIssueHeader::ARROW_INDENT = 6 + 16; //Left margin + arrow;
const int StalledIssueHeader::ICON_INDENT = 8 + 48; // fileIcon + spacer;
const int StalledIssueHeader::BODY_INDENT = StalledIssueHeader::ARROW_INDENT + StalledIssueHeader::ICON_INDENT; // full indent;
const int StalledIssueHeader::GROUPBOX_INDENT = BODY_INDENT - 9;// Following the InVision mockups
const int StalledIssueHeader::GROUPBOX_CONTENTS_INDENT = 9;// Following the InVision mockups
const int StalledIssueHeader::HEIGHT = 60;

const char* MULTIACTION_ICON = "MULTIACTION_ICON";
const char* FILENAME_PROPERTY = "FILENAME_PROPERTY";
const char* MULTIPLE_ACTIONS_PROPERTY = "ACTIONS_PROPERTY";
const char* ISSUE_STATE = "STATE";

StalledIssueHeader::StalledIssueHeader(QWidget* parent):
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader),
    mIsExpandable(true)
{
    ui->setupUi(this);
    connect(ui->actionWaitingSpinner, &WaitingSpinnerWidget::needsUpdate, this, &StalledIssueHeader::needsUpdate);
    connect(ui->multipleActionButton, &QPushButton::clicked, this, &StalledIssueHeader::onMultipleActionClicked);
    ui->fileNameTitle->setTextFormat(Qt::TextFormat::AutoText);
    ui->errorDescriptionText->setTextFormat(Qt::TextFormat::AutoText);
    QSizePolicy sp_retain = ui->arrow->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->arrow->setSizePolicy(sp_retain);
}

StalledIssueHeader::~StalledIssueHeader()
{
    delete ui;
}

void StalledIssueHeader::expand(bool state)
{
    if(mIsExpandable)
    {
        ui->arrow->setChecked(state);
    }
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

//For all issue with ignorable paths which are not special, hard or sym links
void StalledIssueHeader::onIgnoreFileActionClicked()
{
    propagateButtonClick();

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    auto canBeIgnoredChecker = [](const std::shared_ptr<const StalledIssue> issue){
        return StalledIssue::convert<IgnoredStalledIssue>(issue) != nullptr;
    };

    MessageDialogInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok, tr("Apply"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.titleText = tr("Are you sure you want to ignore this issue?");
    msgInfo.descriptionText = tr("This action will ignore this issue and it will not be synced.");

    auto selection = dialog->getDialog()->getSelection(canBeIgnoredChecker);
    auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(canBeIgnoredChecker);
    if(allSimilarIssues.size() != selection.size())
    {
        msgInfo.checkboxText = tr("Apply to all");
    }

    msgInfo.finishFunc = [selection](QPointer<MessageDialogResult> msgBox)
    {
        if (msgBox->result() == QMessageBox::Ok)
        {
            if (msgBox->isChecked())
            {
                MegaSyncApp->getStalledIssuesModel()->ignoreAllSimilarIssues();
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->ignoreItems(selection);
            }
        }
    };

    MessageDialogOpener::warning(msgInfo);
}

void StalledIssueHeader::showIgnoreFile()
{
    StalledIssueHeader::ActionInfo action(tr("Ignore"), ActionsId::Ignore);
    showAction(action);
}

void StalledIssueHeader::propagateButtonClick()
{
    QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::AltModifier));
    qApp->processEvents();
}

void StalledIssueHeader::showAction(const ActionInfo& action)
{
    showActions(QString(), QList<ActionInfo>() << action);
}

void StalledIssueHeader::showActions(const QString &actionButtonText, const QList<ActionInfo>& actions)
{
    bool hasMultipleActions = actions.size() > 1;

    ui->actionContainer->show();
    ui->multipleActionButton->setVisible(true);

    if (hasMultipleActions)
    {
        ui->multipleActionButton->setText(actionButtonText);
    }
    else
    {
        ui->multipleActionButton->setText(actions.first().actionText);
    }

    ui->multipleActionButton->setProperty(MULTIPLE_ACTIONS_PROPERTY, QVariant::fromValue<QList<ActionInfo>>(actions));

    updateMultipleActionButtonIcon();
}

void StalledIssueHeader::hideAction()
{
    ui->multipleActionButton->setVisible(false);
}

void StalledIssueHeader::onMultipleActionClicked()
{
    propagateButtonClick();

    auto actions(getActions());
    if(!actions.isEmpty())
    {
        if(actions.size() == 1)
        {
            if(actions.first().id == ActionsId::Ignore)
            {
                onIgnoreFileActionClicked();
            }
            else
            {
                mHeaderCase->onMultipleActionButtonOptionSelected(this, actions.first().id);
            }
        }
        else
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
}

void StalledIssueHeader::showMessage(const QString& message,
                                     const QString& icon,
                                     QString& iconToken)
{
    ui->actionContainer->show();
    if (!message.isEmpty() || !icon.isEmpty())
    {
        ui->actionMessageContainer->setVisible(true);
    }

    ui->actionMessage->setText(message);

    if (!icon.isEmpty())
    {
        ui->actionMessageIcon->show();
        ui->actionMessageIcon->setIcon(QIcon(icon));
        ui->actionMessageIcon->setProperty(TOKEN_PROPERTIES::normalOff, iconToken);
    }
    else
    {
        ui->actionMessageIcon->hide();
    }
}

void StalledIssueHeader::updateIssueState()
{
    StalledIssue::SolveType type(StalledIssue::SolveType::UNSOLVED);
    QString iconName;
    QString iconToken;
    QString message;

    if (getData().consultData()->hasCustomMessage())
    {
        auto customMessage(getData().consultData()->getCustomMessage());

        type = customMessage.customType;
        message = customMessage.customMessage;
    }
    else
    {
        type = getData().consultData()->getIsSolved();
    }

    if(type == StalledIssue::SolveType::BEING_SOLVED)
    {
        ui->actionWaitingSpinner->start();
        ui->actionWaitingSpinner->setColor(
            TokenParserWidgetManager::instance()->getColor(QLatin1String("icon-primary")));
    }
    else
    {
        ui->actionWaitingSpinner->stop();
    }

    switch(type)
    {
        case StalledIssue::SolveType::BEING_SOLVED:
        {
            ui->actionMessageContainer->setProperty(ISSUE_STATE, QLatin1String("being solved"));
            if (message.isEmpty())
            {
                message = tr("Being solved");
            }
            break;
        }
        case StalledIssue::SolveType::SOLVED:
        {
            if(getData().convert<IgnoredStalledIssue>())
            {
                ui->actionMessageContainer->setProperty(ISSUE_STATE, QLatin1String("ignored"));

                iconName = Utilities::getPixmapName(QLatin1String("minus_circle"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE);
                iconToken = QLatin1String("support_warning");

                if (message.isEmpty())
                {
                    message = tr("Ignored");
                }
            }
            else
            {
                ui->actionMessageContainer->setProperty(ISSUE_STATE, QLatin1String("solved"));

                if(getData().consultData()->wasAutoResolutionApplied())
                {
                    iconName = Utilities::getPixmapName(QLatin1String("magic_wand"),
                                                        Utilities::AttributeType::SMALL |
                                                            Utilities::AttributeType::THIN |
                                                            Utilities::AttributeType::OUTLINE);
                    iconToken = QLatin1String("support_sucess");

                    if (message.isEmpty())
                    {
                        message = tr("Auto-solved");
                    }
                }
                else
                {
                    iconName = Utilities::getPixmapName(QLatin1String("check"),
                                                        Utilities::AttributeType::SMALL |
                                                            Utilities::AttributeType::THIN |
                                                            Utilities::AttributeType::OUTLINE);
                    iconToken = QLatin1String("support_success");

                    if (message.isEmpty())
                    {
                        message = tr("Solved");
                    }
                }
            }

            break;
        }
        case StalledIssue::SolveType::FAILED:
        {
            ui->actionMessageContainer->setProperty(ISSUE_STATE, QLatin1String("failed"));
            iconName = Utilities::getPixmapName(QLatin1String("cross"),
                                                Utilities::AttributeType::SMALL |
                                                    Utilities::AttributeType::THIN |
                                                    Utilities::AttributeType::OUTLINE);
            iconToken = QLatin1String("support_error");

            if (message.isEmpty())
            {
                message = tr("Failed");
            }

            break;
        }
        case StalledIssue::SolveType::UNSOLVED:
        {
            ui->actionMessageContainer->setProperty(ISSUE_STATE, QString());
            if(getData().convert<IgnoredStalledIssue>())
            {
                showIgnoreFile();
            }
            break;
        }
        default:
        {
            break;
        }
    }

    showMessage(message, iconName, iconToken);
    ui->actionMessageContainer->setStyleSheet(ui->actionMessageContainer->styleSheet());
}

void StalledIssueHeader::setText(const QString &text, const QString& tooltip)
{
    if(text.isEmpty())
    {
        ui->fileNameTitle->hide();
    }
    else
    {
        ui->fileNameTitle->show();
        ui->fileNameTitle->setText(text);
        ui->fileNameTitle->setToolTip(tooltip);
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
        //Reset strings first
        setText(QString());
        setTitleDescriptionText(QString());

        mHeaderCase->refreshCaseTitles(this);
    }
}

void StalledIssueHeader::refreshCaseActions()
{
    if(mHeaderCase)
    {
        mHeaderCase->refreshCaseActions(this);
        updateHeaderSizes();
    }
}

void StalledIssueHeader::updateHeaderSizes()
{
    //Why two times? not sure, but works
    for(int times = 0; times < 2; ++times)
    {
        layout()->activate();
        updateGeometry();

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

        ui->fileNameTitle->updateGeometry();
        ui->errorDescriptionText->updateGeometry();
    }
}

QString StalledIssueHeader::fileName()
{
    return QString();
}

bool StalledIssueHeader::event(QEvent* event)
{
    if (event->type() == ThemeManager::ThemeChanged)
    {
        updateMultipleActionButtonIcon();
    }

    return StalledIssueBaseDelegateWidget::event(event);
}

void StalledIssueHeader::refreshUi()
{
    ui->errorTitleIcon->setIcon(QIcon(Utilities::getColoredPixmap(QLatin1String("alert-triangle"),
                                                                  Utilities::AttributeType::NONE,
                                                                  QLatin1String("support-warning"),
                                                                  ui->errorTitleIcon->size())));

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

    if(getData().consultData()->filesCount() > 0)
    {
        fileTypeIcon = Utilities::getCachedPixmap(
            Utilities::getExtensionPixmapName(getData().consultData()->getFileName(false),
                                              Utilities::AttributeType::MEDIUM));
    }
    else
    {
        fileTypeIcon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_NORMAL,
                                                  Utilities::AttributeType::MEDIUM);
    }

    ui->fileTypeIcon->setIcon(fileTypeIcon);

    resetSolvingWidgets();

    updateIssueState();

    //By default it is expandable
    if(getData().consultData())
    {
        setIsExpandable(getData().consultData()->isExpandable());
    }
}

void StalledIssueHeader::resetSolvingWidgets()
{
    ui->multipleActionButton->hide();
    ui->actionMessageContainer->hide();
}

void StalledIssueHeader::updateMultipleActionButtonIcon()
{
    bool hasMultipleActions(getActions().size() > 1);

    if (hasMultipleActions && ui->multipleActionButton->icon().isNull())
    {
        ui->multipleActionButton->setIcon(
            ui->multipleActionButton->property(MULTIACTION_ICON).value<QIcon>());
    }
    else if (!hasMultipleActions && !ui->multipleActionButton->icon().isNull())
    {
        ui->multipleActionButton->setProperty(
            MULTIACTION_ICON,
            QVariant::fromValue<QIcon>(ui->multipleActionButton->icon()));
        ui->multipleActionButton->setIcon(QIcon());
    }
}

QList<StalledIssueHeader::ActionInfo> StalledIssueHeader::getActions()
{
    return ui->multipleActionButton->property(MULTIPLE_ACTIONS_PROPERTY).value<QList<ActionInfo>>();
}

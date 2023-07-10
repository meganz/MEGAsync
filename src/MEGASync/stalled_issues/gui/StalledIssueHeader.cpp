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
    ui->actionMessageContainer->hide();
    ui->ignoreFileButton->hide();
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
    showMessage(tr("Ignored"), icon.pixmap(24,24));

    if(!ui->titleContainer->graphicsEffect())
    {
        auto fileNameEffect = new QGraphicsOpacityEffect(this);
        fileNameEffect->setOpacity(0.30);
        ui->titleContainer->setGraphicsEffect(fileNameEffect);
    }
}

void StalledIssueHeader::clearLabels()
{
    ui->leftTitleText->clear();
    ui->rightTitleText->clear();
    ui->fileNameTitle->clear();

    ui->errorTitleTextContainer->removeEventFilter(this);
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

void StalledIssueHeader::showMessage(const QString &message, const QPixmap& pixmap)
{
    ui->actionMessageContainer->setVisible(true);
    ui->actionMessage->setText(message);

    if(!pixmap.isNull())
    {
        ui->actionMessageIcon->setPixmap(pixmap);
    }
}

void StalledIssueHeader::setLeftTitleText(const QString &text)
{
    ui->leftTitleText->setText(text);
}

void StalledIssueHeader::addFileName(bool preferCloud)
{
    auto fileName = getData().consultData()->getFileName(preferCloud);
    addFileName(fileName);
}

void StalledIssueHeader::addFileName(const QString& filename)
{
    ui->fileNameTitle->setText(filename);
    ui->fileNameTitle->setProperty(FILENAME_PROPERTY, filename);
    ui->errorTitleTextContainer->installEventFilter(this);
}

void StalledIssueHeader::setRightTitleText(const QString &text)
{
    ui->rightTitleText->setText(text);
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
        QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::NoModifier));
        qApp->processEvents();
        mHeaderCase->onActionButtonClicked(this);

        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
        dialog->getDialog()->updateView();
    }
}

void StalledIssueHeader::on_ignoreFileButton_clicked()
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

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

        msgInfo.buttons |= QMessageBox::Yes;
        textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
        textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
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

bool StalledIssueHeader::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->errorTitleTextContainer && event->type() == QEvent::Resize)
    {
        if(!ui->fileNameTitle->text().isEmpty())
        {
            auto filename = ui->fileNameTitle->property(FILENAME_PROPERTY).toString();

            auto blankSpaces = ui->errorTitleTextContainer->layout()->spacing() * 2 + ui->errorTitleTextContainer->contentsMargins().right() + ui->errorTitleTextContainer->contentsMargins().left();
            auto availableWidth = ui->errorTitleTextContainer->width() - ui->rightTitleText->width() - ui->leftTitleText->width() - ui->fileTypeIcon->width() - blankSpaces;
            auto elidedText = ui->fileNameTitle->fontMetrics().elidedText(filename,Qt::ElideMiddle, availableWidth);
            ui->fileNameTitle->setText(elidedText);
        }
    }

    return StalledIssueBaseDelegateWidget::eventFilter(watched, event);
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

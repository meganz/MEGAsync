#include "StalledIssueHeader.h"

#include <stalled_issues/model/StalledIssuesModel.h>

#include <MegaApplication.h>
#include <Preferences.h>

#include "Utilities.h"

#include <QFile>

const int StalledIssueHeader::ARROW_INDENT = 6 + 16; //Left margin + arrow;
const int StalledIssueHeader::ICON_INDENT = 8 + 48; // fileIcon + spacer;
const int StalledIssueHeader::BODY_INDENT = StalledIssueHeader::ARROW_INDENT + StalledIssueHeader::ICON_INDENT; // full indent;
const int StalledIssueHeader::GROUPBOX_INDENT = BODY_INDENT - 9;// Following the InVision mockups
const int StalledIssueHeader::GROUPBOX_CONTENTS_INDENT = 9;// Following the InVision mockups
const int StalledIssueHeader::HEIGHT = 60;

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

void StalledIssueHeader::addFileName()
{
    ui->fileNameTitle->setText(getData().consultData()->getFileName());
    ui->fileNameTitle->installEventFilter(this);
}

void StalledIssueHeader::setRightTitleText(const QString &text)
{
    ui->rightTitleText->setText(text);
}

void StalledIssueHeader::setTitleDescriptionText(const QString &text)
{
    ui->errorDescriptionText->setText(text);
}

QString StalledIssueHeader::fileName()
{
    return QString();
}

void StalledIssueHeader::on_ignoreFileButton_clicked()
{
    auto info = getData().consultData();
    if(info)
    {
        auto ignoredFiles = info->getIgnoredFiles();

        foreach(auto file, ignoredFiles)
        {
            mUtilities.ignoreFile(file);
        }

        MegaSyncApp->getStalledIssuesModel()->solveIssue(false,getCurrentIndex());
        mIsSolved = true;
    }
}

bool StalledIssueHeader::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->fileNameTitle && event->type() == QEvent::Resize)
    {
        auto elidedText = ui->fileNameTitle->fontMetrics().elidedText(getData().consultData()->getFileName(),Qt::ElideMiddle, ui->fileNameTitle->width());
        ui->fileNameTitle->setText(elidedText);
    }

    return StalledIssueBaseDelegateWidget::eventFilter(watched, event);
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
                                                      getData().consultData()->getFileName(), QLatin1Literal(":/images/drag_")));
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

    refreshCaseUi();
}

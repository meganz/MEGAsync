#include "StalledIssueHeader.h"

#include <MegaApplication.h>
#include <Preferences.h>

#include "Utilities.h"

#include <QFile>

const int StalledIssueHeader::ARROW_INDENT = 6 + 16; //Left margin + arrow;
const int StalledIssueHeader::ICON_INDENT = 8 + 48; // fileIcon + spacer;
const int StalledIssueHeader::BODY_INDENT = StalledIssueHeader::ARROW_INDENT + StalledIssueHeader::ICON_INDENT; // full indent;
const int StalledIssueHeader::HEIGHT = 60;

StalledIssueHeader::StalledIssueHeader(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader)
{
    ui->setupUi(this);

    ui->actionButton->hide();
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

void StalledIssueHeader::showAction(const QString &actionButtonText)
{
    ui->actionButton->setVisible(true);
    ui->actionButton->setText(actionButtonText);
}

void StalledIssueHeader::setLeftTitleText(const QString &text)
{
    ui->leftTitleText->setText(text);
}

void StalledIssueHeader::addFileName()
{
    ui->fileNameTitle->setText(getData().getFileName());
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

void StalledIssueHeader::ignoreFile()
{
    auto data = getData().getStalledIssueData();
    if(data)
    {
        auto path = data->mPath.path;

        connect(&mIgnoreWatcher, &QFutureWatcher<void>::finished,
                this, &StalledIssueHeader::onIgnoreFileFinished);

        QFuture<void> addToIgnore = QtConcurrent::run([path]()
        {
            QFileInfo stalledIssuePathInfo(path);
            QDir ignoreDir(path);

            while(ignoreDir.exists())
            {
                QFile ignore(ignoreDir.path() + QDir::separator() + QString::fromUtf8(".megaignore"));
                if(ignore.exists())
                {
                    ignore.open(QFile::Append | QFile::Text);

                    QTextStream streamIn(&ignore);
                    streamIn << QChar((int)'\n');

                    streamIn << QString::fromUtf8("-:");

                    streamIn << ignoreDir.relativeFilePath(path);
                    ignore.close();

                    break;
                }

                if(!ignoreDir.cdUp())
                {
                    break;
                }
            }
        });

        mIgnoreWatcher.setFuture(addToIgnore);
    }
}

QString StalledIssueHeader::fileName()
{
return QString();
}

void StalledIssueHeader::onIgnoreFileFinished()
{
    emit issueFixed();
    disconnect(&mIgnoreWatcher, &QFutureWatcher<void>::finished,
               this, &StalledIssueHeader::onIgnoreFileFinished);
}

bool StalledIssueHeader::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->fileNameTitle && event->type() == QEvent::Resize)
    {
        auto elidedText = ui->fileNameTitle->fontMetrics().elidedText(getData().getFileName(),Qt::ElideMiddle, ui->fileNameTitle->width());
        ui->fileNameTitle->setText(elidedText);
    }

    return StalledIssueBaseDelegateWidget::eventFilter(watched, event);
}

void StalledIssueHeader::refreshUi()
{
    auto errorTitleIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/ico_menu_full.png"));
    ui->errorTitleIcon->setPixmap(errorTitleIcon.pixmap(ui->errorTitleIcon->size()));

    QIcon fileTypeIcon;

    auto splittedFile = getData().getFileName().split(QString::fromUtf8("."));
    if(splittedFile.size() != 1)
    {
        fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                      getData().getFileName(), QLatin1Literal(":/images/drag_")));
    }
    else
    {
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/color_folder@2x.png"));
    }

    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));

    refreshCaseUi();
}

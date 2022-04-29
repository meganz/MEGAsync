#include "StalledIssueHeader.h"

#include <MegaApplication.h>
#include <Preferences.h>

#include "Utilities.h"

#include <QFile>

const int StalledIssueHeader::ARROW_INDENT = 6 + 16; //Left margin + arrow;
const int StalledIssueHeader::ICON_INDENT = 8 + 48; // fileIcon + spacer;
const int StalledIssueHeader::BODY_INDENT = StalledIssueHeader::ARROW_INDENT + StalledIssueHeader::ICON_INDENT; // full indent;

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

void StalledIssueHeader::showAction()
{
    ui->actionButton->setVisible(true);
}

void StalledIssueHeader::ignoreFile()
{
    auto data = getData().getStalledIssueData();
    if(data)
    {
        auto path = data->mIndexPath.path;

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

                    if(stalledIssuePathInfo.isFile())
                    {
                        streamIn << QString::fromUtf8("-f:");
                    }
                    else
                    {
                        streamIn << QString::fromUtf8("-dp:");
                    }

                    streamIn << ignoreDir.relativeFilePath(stalledIssuePathInfo.path());
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

void StalledIssueHeader::onIgnoreFileFinished()
{
    emit issueFixed();
    disconnect(&mIgnoreWatcher, &QFutureWatcher<void>::finished,
            this, &StalledIssueHeader::onIgnoreFileFinished);
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
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/color_folder.png"));
    }

    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));

    refreshCaseUi();
}

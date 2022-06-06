#include "StalledIssueFilePath.h"
#include "ui_StalledIssueFilePath.h"

#include "Utilities.h"
#include "Platform.h"
#include "QMegaMessageBox.h"

#include <QPainter>
#include <QPoint>

static const char* ITS_HOVER = "ItsHover";
static const char* HAS_PROBLEM = "hasProblem";

StalledIssueFilePath::StalledIssueFilePath(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueFilePath),
    mShowFullPath(false)
{
    ui->setupUi(this);

    ui->filePathAction->hide();
    ui->moveFilePathAction->hide();

    ui->file->hide();
    ui->moveFile->hide();

    ui->lines->installEventFilter(this);
    ui->moveLines->installEventFilter(this);
    ui->movePathProblemLines->installEventFilter(this);

    ui->filePathContainer->installEventFilter(this);
    ui->moveFilePathContainer->installEventFilter(this);

    auto openIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/ic-open-outside.png"));
    ui->filePathAction->setPixmap(openIcon.pixmap(ui->filePathAction->size()));
    ui->moveFilePathAction->setPixmap(openIcon.pixmap(ui->moveFilePathAction->size()));
}

StalledIssueFilePath::~StalledIssueFilePath()
{
    delete ui;
}

void StalledIssueFilePath::setIndent(int indent)
{
    ui->indent->changeSize(indent,0,QSizePolicy::Fixed, QSizePolicy::Preferred);
    ui->gridLayout->invalidate();
}

void StalledIssueFilePath::updateUi(StalledIssueDataPtr data)
{
    mData = data;

    if(mData->isCloud())
    {
        auto remoteIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/cloud_default.png"));
        ui->LocalOrRemoteIcon->setPixmap(remoteIcon.pixmap(QSize(16,16)));

        ui->LocalOrRemoteText->setText(tr("on MEGA:"));
    }
    else
    {
        auto localIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/monitor_default.png"));
        ui->LocalOrRemoteIcon->setPixmap(localIcon.pixmap(QSize(16,16)));

        ui->LocalOrRemoteText->setText(tr("Local:"));
    }

    fillFilePath();
    fillMoveFilePath();
    updateFileIcons();
    updateMoveFileIcons();
}

void StalledIssueFilePath::showFullPath()
{
    mShowFullPath = true;
}

void StalledIssueFilePath::fillFilePath()
{
    if(!mData->getPath().isEmpty())
    {
        ui->file->show();

        auto filePath = getFilePath();
        if(!filePath.isEmpty())
        {
            ui->filePath->installEventFilter(this);
            ui->filePath->setText(filePath);
        }
        else
        {
            ui->filePath->setText(QString::fromUtf8("-"));
        }

        auto hasProblem(mData->getPath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem);
        hasProblem ?  ui->pathProblemMessage->setText(getSyncPathProblemString(mData->getPath().mPathProblem)) : ui->pathProblemContainer->hide();
        hasProblem ? ui->filePathContainer->setCursor(Qt::ArrowCursor) : ui->filePathContainer->setCursor(Qt::PointingHandCursor);

        ui->filePathContainer->setProperty(HAS_PROBLEM,hasProblem);
        setStyleSheet(styleSheet());
    }
}

QString StalledIssueFilePath::getFilePath()
{
    return mShowFullPath? mData->getNativeFilePath() : mData->getNativePath();
}

void StalledIssueFilePath::fillMoveFilePath()
{
    if(!mData->getMovePath().isEmpty())
    {
        ui->moveFile->show();

        auto filePath = getMoveFilePath();
        if(!filePath.isEmpty())
        {
            ui->moveFilePath->installEventFilter(this);
            ui->moveFilePath->setText(filePath);
        }
        else
        {
            ui->moveFilePath->setText(QString::fromUtf8("-"));
        }

        auto hasProblem(mData->getMovePath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem);
        hasProblem ?  ui->movePathProblemMessage->setText(getSyncPathProblemString(mData->getMovePath().mPathProblem)) : ui->movePathProblemContainer->hide();
        hasProblem ? ui->moveFilePathContainer->setCursor(Qt::ArrowCursor) : ui->moveFilePathContainer->setCursor(Qt::PointingHandCursor);
        ui->moveFilePathContainer->setProperty(HAS_PROBLEM,hasProblem);
        setStyleSheet(styleSheet());
    }
}

QString StalledIssueFilePath::getMoveFilePath()
{
    return mShowFullPath? mData->getNativeMoveFilePath() : mData->getNativeMovePath();
}

void StalledIssueFilePath::updateFileIcons()
{
    QFileInfo fileInfo(getFilePath());
    auto hasProblem(mData->getPath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem);
    QIcon fileTypeIcon(getPathIcon(fileInfo, hasProblem));

    ui->filePathIcon->setPixmap(fileTypeIcon.pixmap(ui->filePathIcon->size()));
}

void StalledIssueFilePath::updateMoveFileIcons()
{
    QFileInfo fileInfo(getMoveFilePath());
    auto hasProblem(mData->getMovePath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem);
    QIcon fileTypeIcon(getPathIcon(fileInfo, hasProblem));

    ui->moveFilePathIcon->setPixmap(fileTypeIcon.pixmap(ui->moveFilePathIcon->size()));
}

QIcon StalledIssueFilePath::getPathIcon(const QFileInfo &fileInfo, bool hasProblem)
{
    QIcon fileTypeIcon;

    bool isFile(false);

    if(fileInfo.exists())
    {
        isFile = fileInfo.isFile();
    }
    else
    {
        isFile = !fileInfo.completeSuffix().isEmpty();
    }

    if(isFile)
    {
        //Without extension
        if(fileInfo.completeSuffix().isEmpty())
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/help-circle.png"));
        }
        else
        {
            fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                          fileInfo.fileName(), QLatin1Literal(":/images/drag_")));
        }
    }
    else
    {
        if(hasProblem)
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/folder_error_default.png"));
        }
        else
        {
            fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/folder_orange_default.png"));
        }

    }

    return fileTypeIcon;
}

bool StalledIssueFilePath::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Enter || event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonRelease)
    {
        if(watched == ui->filePathContainer && !ui->filePathContainer->property(HAS_PROBLEM).toBool())
        {
            showHoverAction(event->type(), ui->filePathAction, getFilePath());
        }
        else if(watched == ui->moveFilePathContainer && !ui->moveFilePathContainer->property(HAS_PROBLEM).toBool())
        {
            showHoverAction(event->type(), ui->moveFilePathAction,  getMoveFilePath());
        }
    }
    else if(event->type() == QEvent::Resize)
    {
        if(watched == ui->filePath || watched == ui->moveFilePath)
        {
            auto label = dynamic_cast<QLabel*>(watched);
            if(label)
            {
                QString fullPath;

                if(watched == ui->filePath)
                {
                    fullPath = getFilePath();
                }
                else if(watched == ui->moveFilePath)
                {
                    fullPath = getMoveFilePath();
                }

                if(!fullPath.isEmpty())
                {
                    label->setText(label->fontMetrics().elidedText(fullPath, Qt::ElideMiddle,label->width()));
                }
            }
        }
        else if(watched == ui->lines)
        {
            auto hasProblem(mData->getPath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem);
            if(hasProblem)
            {
                auto fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/tree_link_end_default.png"));
                ui->lines->setPixmap(fileTypeIcon.pixmap(ui->lines->size()));
            }
            else
            {
                auto fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/tree_end_default.png"));
                ui->lines->setPixmap(fileTypeIcon.pixmap(ui->lines->size()));
            }
        }
        else if(watched == ui->movePathProblemLines)
        {
            auto hasProblem(mData->getMovePath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem);
            if(hasProblem)
            {
                auto fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/tree_double_link_default.png"));
                ui->movePathProblemLines->setPixmap(fileTypeIcon.pixmap(ui->movePathProblemLines->size()));
            }
            else
            {
                ui->movePathProblemLines->setPixmap(QPixmap());
            }
        }
        else if(watched == ui->moveLines)
        {
            auto fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/tree_link_default.png"));
            ui->moveLines->setPixmap(fileTypeIcon.pixmap(ui->moveLines->size()));
        }
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueFilePath::showHoverAction(QEvent::Type type, QWidget *actionWidget, const QString& path)
{
    if(type == QEvent::Enter)
    {
        actionWidget->show();
        actionWidget->parent()->setProperty(ITS_HOVER, true);
        setStyleSheet(styleSheet());
    }
    else if(type == QEvent::Leave)
    {
        actionWidget->hide();
        actionWidget->parent()->setProperty(ITS_HOVER, false);
        setStyleSheet(styleSheet());
    }
    else if(type == QEvent::MouseButtonRelease)
    {
        if(mData->isCloud())
        {
            mega::MegaNode* node (MegaSyncApp->getMegaApi()->getNodeByPath(path.toStdString().c_str()));
            if (node)
            {
                const char* handle = node->getBase64Handle();
                QString url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);
                QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
                delete [] handle;
                delete node;
            }
            else
            {
                QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("Node %1 does not exist.").arg(path));
            }
        }
        else
        {
            QFile file(path);
            if(file.exists())
            {
                QtConcurrent::run([=]
                {
                    Platform::showInFolder(path);
                });
            }
            else
            {
                QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("Path %1 does not exist.").arg(path));
            }
        }
    }
}

QString StalledIssueFilePath::getSyncPathProblemString(mega::MegaSyncStall::SyncPathProblem pathProblem)
{
    switch(pathProblem)
    {
        case mega::MegaSyncStall::FileChangingFrequently:
        {
            return tr("File is being frequently changing.");
            break;
        }
        case mega::MegaSyncStall::IgnoreRulesUnknown:
        {
            return tr("Ignore rules unknown.");
            break;
        }
        case mega::MegaSyncStall::DetectedHardLink:
        {
            return tr("Hard link detected.");
            break;
        }
        case mega::MegaSyncStall::DetectedSymlink:
        {
            return tr("Detected Sym link.");
            break;
        }
        case mega::MegaSyncStall::DetectedSpecialFile:
        {
            return tr("Detected special file.");
            break;
        }
        case mega::MegaSyncStall::DifferentFileOrFolderIsAlreadyPresent:
        {
            return tr("Different file or folder is already present.");
            break;
        }
        case mega::MegaSyncStall::ParentFolderDoesNotExist:
        {
            return tr("Parent folder does not exist.");
            break;
        }
        case mega::MegaSyncStall::FilesystemErrorDuringOperation:
        {
            return tr("Filesystem error during operation.");
            break;
        }
        case mega::MegaSyncStall::NameTooLongForFilesystem:
        {
            return tr("Name too long for filesystem.");
            break;
        }
        case mega::MegaSyncStall::CannotFingrprintFile:
        {
            return tr("Cannot fingerprint file.");
            break;
        }
        case mega::MegaSyncStall::DestinationPathInUnresolvedArea:
        {
            return tr("Destination path is in an unresolved area.");
            break;
        }
        case mega::MegaSyncStall::MACVerificationFailure:
        {
            return tr("MAC verification failure.");
            break;
        }
        case mega::MegaSyncStall::DeletedOrMovedByUser:
        {
            return tr("Deleted or moved by user.");
            break;
        }
        case mega::MegaSyncStall::FileFolderDeletedByUser:
        {
            return tr("Deleted by user.");
            break;
        }
        case mega::MegaSyncStall::MoveToDebrisFolderFailed:
        {
            return tr("Move to debris folder failed.");
            break;
        }
        case mega::MegaSyncStall::IgnoreFileMalformed:
        {
            return tr("Ingore file malformed.");
            break;
        }
        case mega::MegaSyncStall::FilesystemErrorListingFolder:
        {
            return tr("Error Listing folder in filesystem.");
            break;
        }
        case mega::MegaSyncStall::FilesystemErrorIdentifyingFolderContent:
        {
            return tr("Error identifying folder content in filesystem.");
            break;
        }
        case mega::MegaSyncStall::UndecryptedCloudNode:
        {
            return tr("Cloud node undecrypted.");
            break;
        }
        default:
            return tr("Error not detected");
    }
}

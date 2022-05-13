#include "StalledIssueFilePath.h"
#include "ui_StalledIssueFilePath.h"

#include "Utilities.h"
#include "Platform.h"
#include "QMegaMessageBox.h"

#include <QPainter>
#include <QPoint>

StalledIssueFilePath::StalledIssueFilePath(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueFilePath)
{
    ui->setupUi(this);

    ui->filePathAction->hide();
    ui->moveFilePathAction->hide();

    ui->file->hide();
    ui->moveFile->hide();

    ui->moveLines->installEventFilter(this);
    ui->lines->installEventFilter(this);

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

void StalledIssueFilePath::updateUi(QExplicitlySharedDataPointer<StalledIssueData> data)
{
    mData = data;

    if(mData->isCloud())
    {
        auto remoteIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/cloud_upload_item_ico.png"));
        ui->LocalOrRemoteIcon->setPixmap(remoteIcon.pixmap(ui->LocalOrRemoteIcon->size()));

        ui->LocalOrRemoteText->setText(tr("on MEGA:"));
    }
    else
    {
        auto localIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/PC_ico_rest.png"));
        ui->LocalOrRemoteIcon->setPixmap(localIcon.pixmap(ui->LocalOrRemoteIcon->size()));

        ui->LocalOrRemoteText->setText(tr("Local:"));
    }

    fillFilePath();
    fillMoveFilePath();
    updateFileIcons();
}

void StalledIssueFilePath::fillFilePath()
{
    if(!mData->getPath().isEmpty())
    {
        ui->file->show();

        auto filePath = mData->getNativePath();
        if(!filePath.isEmpty())
        {
            ui->filePath->installEventFilter(this);
            ui->filePath->setText(mData->getNativePath());
        }
        else
        {
            ui->filePath->setText(QString::fromUtf8("-"));
        }

        mData->getPath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem
                ?  ui->pathProblem->setText(getSyncPathProblemString(mData->getPath().mPathProblem)) : ui->pathProblem->hide();
    }
}

void StalledIssueFilePath::fillMoveFilePath()
{
    if(!mData->getMovePath().isEmpty())
    {
        ui->moveFile->show();

        auto filePath = mData->getNativeMovePath();
        if(!filePath.isEmpty())
        {
            ui->moveFilePath->installEventFilter(this);
            ui->moveFilePath->setText(mData->getNativePath());
        }
        else
        {
            ui->moveFilePath->setText(QString::fromUtf8("-"));
        }

        mData->getMovePath().mPathProblem != mega::MegaSyncStall::SyncPathProblem::NoProblem
                ?  ui->movePathProblem->setText(getSyncPathProblemString(mData->getMovePath().mPathProblem)) : ui->movePathProblem->hide();
    }
}

void StalledIssueFilePath::updateFileIcons()
{
    QIcon fileTypeIcon;
    QFileInfo fileInfo(mData->getFileName());

    if(fileInfo.isFile())
    {
        //Without extension
        if(mData->getFileName() == fileInfo.baseName())
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
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/color_folder.png"));
    }


    ui->filePathIcon->setPixmap(fileTypeIcon.pixmap(ui->filePathIcon->size()));
    ui->moveFilePathIcon->setPixmap(fileTypeIcon.pixmap(ui->moveFilePathIcon->size()));
}

bool StalledIssueFilePath::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lines && event->type() == QEvent::Paint)
    {
        QPainter p(ui->lines);
        p.setPen(QPen(QColor("#000000"),1));
        p.setOpacity(0.2);

        auto width(ui->lines->width());
        auto height(ui->lines->height());

        p.drawLine(QPoint(width/2,0), QPoint(width/2, ui->lines->height()/2));
        p.drawLine(QPoint(width/2,height/2), QPoint(width - 2, height/2));
    }
    else if(watched == ui->moveLines && event->type() == QEvent::Paint)
    {
        QPainter p(ui->moveLines);
        p.setPen(QPen(QColor("#000000"),1));
        p.setOpacity(0.2);

        auto width(ui->moveLines->width());
        auto height(ui->moveLines->height());

        p.drawLine(QPoint(0,height/2), QPoint(width, height/2));
        p.drawLine(QPoint(0,0), QPoint(0, ui->moveLines->height()));
    }
    else if(event->type() == QEvent::Enter || event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonRelease)
    {
        if(watched == ui->filePathContainer)
        {
            showHoverAction(event->type(), ui->filePathAction, mData->getNativeFilePath());
        }
        else if(watched == ui->moveFilePathContainer)
        {
            showHoverAction(event->type(), ui->moveFilePathAction,  mData->getNativeMoveFilePath());
        }
    }
    else if(event->type() == QEvent::Resize)
    {
        if(auto label = dynamic_cast<QLabel*>(watched))
        {
            QString fullPath;

            if(label == ui->filePath)
            {
                fullPath = mData->getNativePath();
            }
            else if(label == ui->moveFilePath)
            {
                fullPath = mData->getNativeMovePath();
            }

            if(!fullPath.isEmpty())
            {
                label->setText(label->fontMetrics().elidedText(fullPath, Qt::ElideMiddle,label->width()));
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueFilePath::showHoverAction(QEvent::Type type, QWidget *actionWidget, const QString& path)
{
    if(type == QEvent::Enter)
    {
        actionWidget->show();
        actionWidget->parent()->setProperty("itsHover", true);
        setStyleSheet(styleSheet());
    }
    else if(type == QEvent::Leave)
    {
        actionWidget->hide();
        actionWidget->parent()->setProperty("itsHover", false);
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

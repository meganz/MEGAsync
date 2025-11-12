#include "StalledIssueFilePath.h"

#include "MegaApplication.h"
#include "ServiceUrls.h"
#include "StalledIssuesUtilities.h"
#include "TokenizableItems/TokenPropertyNames.h"
#include "ui_StalledIssueFilePath.h"
#include "Utilities.h"

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

    // hover action button are by default invisible
    ui->filePathAction->hide();
    ui->moveFilePathAction->hide();

    connect(ui->helpIcon, &QPushButton::clicked, this, &StalledIssueFilePath::onHelpIconClicked);
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

void StalledIssueFilePath::updateUi(StalledIssueDataPtr newData)
{
    if(!mData && newData)
    {
        ui->filePathContainer->installEventFilter(this);
        ui->moveFilePathContainer->installEventFilter(this);
    }
    else if(!newData)
    {
        ui->filePathContainer->removeEventFilter(this);
        ui->moveFilePathContainer->removeEventFilter(this);
    }

    mData = newData;

    updateFileIcons();
    updateMoveFileIcons();
    fillFilePath();
    fillMoveFilePath();
    updateCornerArrows();
    updateLocalOrMegaTitle();
}

void StalledIssueFilePath::showFullPath()
{
    mShowFullPath = true;
}

void StalledIssueFilePath::hideLocalOrRemoteTitle()
{
    ui->LocalOrRemoteIcon->hide();
    ui->LocalOrRemoteText->hide();
}

void StalledIssueFilePath::fillFilePath()
{
    if(!mData->getPath().isEmpty())
    {
        ui->file->show();
        const auto pathProblem(mData->getPath().pathProblem);

        auto hasProblem(showError(pathProblem));

        if(hasProblem)
        {
            ui->pathProblemContainer->show();
            ui->pathProblemArrowContainer->show();

            ui->pathProblemMessage->setText(getSyncPathProblemString(pathProblem));
            ui->filePathContainer->setCursor(Qt::ArrowCursor);
            auto helpLink = getHelpLink(pathProblem);
            helpLink.isEmpty() ? ui->helpIcon->hide() : ui->helpIcon->show();
        }
        else
        {
            ui->pathProblemContainer->hide();
            ui->pathProblemArrowContainer->hide();

            ui->filePathContainer->setCursor(Qt::PointingHandCursor);
            ui->helpIcon->hide();
        }

        ui->filePathContainer->setProperty(HAS_PROBLEM,hasProblem);
        setStyleSheet(styleSheet());

        auto filePath = getFilePath();
        if(!filePath.isEmpty())
        {
            ui->filePathEdit->setText(filePath);
        }
        else
        {
            ui->filePathEdit->setText(QString::fromUtf8("-"));
        }
    }
    else
    {
        ui->file->hide();
    }
}

QString StalledIssueFilePath::getFilePath() const
{
    QString filePath;
    if(mData)
    {
        QFileInfo fileInfo(mShowFullPath? mData->getNativeFilePath() : mData->getNativePath());
        filePath = mData->getPath().showDirectoryInHyperLink ? fileInfo.path() : fileInfo.filePath();
        mData->checkTrailingSpaces(filePath);
    }
    return filePath;
}

void StalledIssueFilePath::fillMoveFilePath()
{
    if(!mData->getMovePath().isEmpty())
    {
        auto hasProblem(showError(mData->getMovePath().pathProblem));
        if (hasProblem)
        {
            ui->movePathProblemContainer->show();
            ui->movePathProblemArrowContainer->show();

            ui->movePathProblemMessage->setText(
                getSyncPathProblemString(mData->getMovePath().pathProblem));
            ui->moveFilePathContainer->setCursor(Qt::ArrowCursor);
        }
        else
        {
            ui->movePathProblemContainer->hide();
            ui->movePathProblemArrowContainer->hide();

            ui->moveFilePathContainer->setCursor(Qt::PointingHandCursor);
        }

        ui->moveFilePathContainer->setProperty(HAS_PROBLEM,hasProblem);
        setStyleSheet(styleSheet());

        ui->moveFile->show();

        auto filePath = getMoveFilePath();
        if(!filePath.isEmpty())
        {
            ui->moveFilePathEdit->setText(filePath);
        }
        else
        {
            ui->moveFilePathEdit->setText(QString::fromUtf8("-"));
        }
    }
    else
    {
        ui->moveFile->hide();
    }
}

QString StalledIssueFilePath::getMoveFilePath() const
{
    QFileInfo fileInfo(mShowFullPath? mData->getNativeMoveFilePath() : mData->getNativeMovePath());
    return mData->getMovePath().showDirectoryInHyperLink ? fileInfo.path() : fileInfo.filePath();
}

std::unique_ptr<mega::MegaNode> StalledIssueFilePath::getNode() const
{
    if(auto cloudIssue = mData->convert<CloudStalledIssueData>())
    {
        if(cloudIssue->getPathHandle() != mega::INVALID_HANDLE)
        {
            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudIssue->getPathHandle()));
            if(node)
            {
                if(mShowFullPath)
                {
                    return node;
                }
                else
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                    return parentNode;
                }
            }
        }

        QFileInfo fileInfo(getFilePath());
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(fileInfo.filePath().toUtf8().constData()));
        return node;
    }

    return nullptr;
}

std::unique_ptr<mega::MegaNode> StalledIssueFilePath::getMoveNode() const
{
    if(auto cloudIssue = mData->convert<CloudStalledIssueData>())
    {
        if(cloudIssue->getPathHandle() != mega::INVALID_HANDLE)
        {
            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(cloudIssue->getMovePathHandle()));
            if(node)
            {
                if(mShowFullPath)
                {
                    return node;
                }
                else
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                    return parentNode;
                }
            }
        }

        QFileInfo fileInfo(getMoveFilePath());
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(fileInfo.filePath().toUtf8().constData()));
        return node;
    }

    return nullptr;
}

void StalledIssueFilePath::updateFileIcons()
{
    QString fileTypeIcon;
    QSize iconSize(ui->filePathIcon->size());

    QFileInfo fileInfo(getFilePath());
    if(mData->isCloud())
    {
        auto node(getNode());
        fileTypeIcon = StalledIssuesUtilities::getRemoteFileIcon(node.get(), fileInfo);
        if(!node)
        {
            iconSize = QSize(16,16);
        }
    }
    else
    {
        fileTypeIcon = StalledIssuesUtilities::getLocalFileIcon(fileInfo);
        if (!fileInfo.exists())
        {
            iconSize = QSize(16, 16);
        }
    }

    // Has a problem
    if (showError(mData->getPath().pathProblem))
    {
        ui->filePathIcon->setProperty(TOKEN_PROPERTIES::normalOff,
                                      QLatin1String("icon-inverse-accent"));
    }
    else
    {
        ui->filePathIcon->setProperty(TOKEN_PROPERTIES::normalOff, QString());
    }

    ui->filePathIcon->setIconSize(iconSize);
    ui->filePathIcon->setIcon(QIcon(fileTypeIcon));
}

void StalledIssueFilePath::updateMoveFileIcons()
{
    QString fileTypeIcon;
    QSize iconSize(ui->moveFilePathIcon->size());

    QFileInfo fileInfo(getMoveFilePath());

    if(mData->isCloud())
    {
        auto node(getMoveNode());

        fileTypeIcon = StalledIssuesUtilities::getRemoteFileIcon(node.get(), fileInfo);
        if(!node)
        {
            iconSize = QSize(16,16);
        }
    }
    else
    {
        fileTypeIcon = StalledIssuesUtilities::getLocalFileIcon(fileInfo);
        if (!fileInfo.exists())
        {
            iconSize = QSize(16, 16);
        }
    }

    // Has a problem
    if (showError(mData->getMovePath().pathProblem))
    {
        ui->moveFilePathIcon->setProperty(TOKEN_PROPERTIES::normalOff,
                                          QLatin1String("icon-inverse-accent"));
    }
    else
    {
        ui->moveFilePathIcon->setProperty(TOKEN_PROPERTIES::normalOff, QString());
    }

    ui->moveFilePathIcon->setIconSize(iconSize);
    ui->moveFilePathIcon->setIcon(QIcon(fileTypeIcon));
}

void StalledIssueFilePath::updateCornerArrows()
{
    // ui->pathArrow
    {
        QIcon icon(Utilities::getIcon(QLatin1String("arrow_corner_right"),
                                      Utilities::AttributeType::NONE));
        ui->pathArrow->setIcon(icon);
    }

    // ui->problemArrow
    {
        QIcon icon(Utilities::getIcon(QLatin1String("arrow_corner_right"),
                                      Utilities::AttributeType::NONE));
        auto hasProblem(showError(mData->getPath().pathProblem));
        if (hasProblem)
        {
            ui->problemArrow->setIcon(icon);
        }
        else
        {
            ui->problemArrow->setIcon(QIcon());
        }
    }

    // ui->movePathArrow
    {
        QIcon icon(Utilities::getIcon(QLatin1String("arrow_corner_right"),
                                      Utilities::AttributeType::NONE));
        ui->movePathArrow->setIcon(icon);
    }

    // ui->movePathProblemArrow
    {
        QIcon icon(Utilities::getIcon(QLatin1String("arrow_corner_right"),
                                      Utilities::AttributeType::NONE));
        auto hasProblem(showError(mData->getMovePath().pathProblem));
        if (hasProblem)
        {
            ui->movePathProblemArrow->setIcon(icon);
        }
        else
        {
            ui->movePathProblemArrow->setIcon(QIcon());
        }
    }
}

void StalledIssueFilePath::updateLocalOrMegaTitle()
{
    QIcon icon;

    if (mData->isCloud())
    {
        icon = Utilities::getIcon(QLatin1String("MEGA"),
                                  Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                      Utilities::AttributeType::OUTLINE);

        ui->LocalOrRemoteText->setText(tr("on MEGA:"));
    }
    else
    {
        icon = Utilities::getIcon(QLatin1String("monitor"),
                                  Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                      Utilities::AttributeType::OUTLINE);

        ui->LocalOrRemoteText->setText(tr("Local:"));
    }

    ui->LocalOrRemoteIcon->setIcon(icon);
}

bool StalledIssueFilePath::eventFilter(QObject *watched, QEvent *event)
{
    //Not needed but used as extra protection
    if(mData)
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
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueFilePath::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void StalledIssueFilePath::onHelpIconClicked()
{
    auto helpLink = getHelpLink(mData->getPath().pathProblem);
    Utilities::openUrl(helpLink);
}

void StalledIssueFilePath::showHoverAction(QEvent::Type type,
                                           QWidget* actionWidget,
                                           const QString& path)
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
        StalledIssuesUtilities::openLink(mData->isCloud(), path);
    }
}

QString StalledIssueFilePath::getSyncPathProblemString(mega::MegaSyncStall::SyncPathProblem pathProblem)
{
    switch(pathProblem)
    {
        case mega::MegaSyncStall::FileChangingFrequently:
        {
            return tr("File is being frequently changing.");
        }
        case mega::MegaSyncStall::IgnoreRulesUnknown:
        {
            return tr("Ignore rules unknown.");
        }
        case mega::MegaSyncStall::DetectedHardLink:
        {
            return tr("Hard link or Reparse Point detected.");
        }
        case mega::MegaSyncStall::DetectedSymlink:
        {
            return tr("Detected Sym link.");
        }
        case mega::MegaSyncStall::DetectedSpecialFile:
        {
            return tr("Detected special file.");
        }
        case mega::MegaSyncStall::DifferentFileOrFolderIsAlreadyPresent:
        {
            return tr("Different file or folder is already present.");
        }
        case mega::MegaSyncStall::ParentFolderDoesNotExist:
        {
            return tr("Parent folder does not exist.");
        }
        case mega::MegaSyncStall::FilesystemErrorDuringOperation:
        {
            return tr("Filesystem error during operation.");
        }
        case mega::MegaSyncStall::NameTooLongForFilesystem:
        {
            return tr("Name too long for filesystem.");
        }
        case mega::MegaSyncStall::CannotFingerprintFile:
        {
            return tr("Cannot fingerprint file.");
        }
        case mega::MegaSyncStall::DestinationPathInUnresolvedArea:
        {
            return tr("The folder could not be found. Ensure that the path is correct and try again.");
        }
        case mega::MegaSyncStall::MACVerificationFailure:
        {
            return tr("MAC verification failure.");
        }
        case mega::MegaSyncStall::DeletedOrMovedByUser:
        {
            return tr("Deleted or moved by user.");
        }
        case mega::MegaSyncStall::FileFolderDeletedByUser:
        {
            return tr("Deleted by user.");
        }
        case mega::MegaSyncStall::MoveToDebrisFolderFailed:
        {
            return tr("Move to debris folder failed.");
        }
        case mega::MegaSyncStall::IgnoreFileMalformed:
        {
            return tr("Ignore file malformed.");
        }
        case mega::MegaSyncStall::FilesystemErrorListingFolder:
        {
            return tr("Error Listing folder in filesystem.");
        }
        case mega::MegaSyncStall::WaitingForScanningToComplete:
        {
            return tr("Waiting for scanning to complete.");
        }
        case mega::MegaSyncStall::WaitingForAnotherMoveToComplete:
        {
            return tr("Waiting for another move to complete.");
        }
        case mega::MegaSyncStall::SourceWasMovedElsewhere:
        {
            return tr("Source was moved elsewhere.");
        }
        case mega::MegaSyncStall::FilesystemCannotStoreThisName:
        {
            return tr("Local filesystem cannot store this name.");
        }
        case mega::MegaSyncStall::CloudNodeInvalidFingerprint:
        {
            return tr("Fingerprint is missing or invalid.");
        }
        case mega::MegaSyncStall::SyncPathProblem_LastPlusOne:
        {
            break;
        }
        default:
            break;
    }
    return tr("Error not detected");
}

bool StalledIssueFilePath::showError(mega::MegaSyncStall::SyncPathProblem pathProblem)
{
    switch (pathProblem)
    {
        case mega::MegaSyncStall::CloudNodeIsBlocked:
        case mega::MegaSyncStall::NoProblem:
        {
            return false;
        }
        default:
        {
            return true;
        }
    }
}

QUrl StalledIssueFilePath::getHelpLink(mega::MegaSyncStall::SyncPathProblem pathProblem)
{
    switch(pathProblem)
    {
        case mega::MegaSyncStall::FilesystemCannotStoreThisName:
        {
            return ServiceUrls::getHelpBaseUrl();
        }
        default:
            break;
    }

    return {};
}

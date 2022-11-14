#include "DuplicatedNodeItem.h"
#include "ui_DuplicatedNodeItem.h"

#include <Utilities.h>
#include <MegaApplication.h>

/*
 * BASE CLASS
*/
DuplicatedNodeItem::DuplicatedNodeItem(QWidget *parent) :
    QWidget(parent),
    mInfo(nullptr),
    mNodeSize(-1),
    ui(new Ui::DuplicatedNodeItem),
    mModifiedTimeVisible(true)
{
    ui->setupUi(this);
    ui->lNodeName->installEventFilter(this);
    ui->lLearnMore->hide();

    connect(&mFolderSizeFuture, &QFutureWatcher<qint64>::finished, this, &DuplicatedLocalItem::onNodeSizeFinished);
}

DuplicatedNodeItem::~DuplicatedNodeItem()
{
    delete ui;
}

void DuplicatedNodeItem::setInfo(std::shared_ptr<DuplicatedNodeInfo>info, NodeItemType type)
{
    mInfo = info;
    connect(mInfo.get(), &DuplicatedNodeInfo::localModifiedDateUpdated, this, &DuplicatedNodeItem::updateModificationTime);

    mType = type;

    fillUi();
}

void DuplicatedNodeItem::setDescription(const QString &description)
{
    ui->lDescription->setText(description);
}

void DuplicatedNodeItem::showLearnMore(const QString& url)
{
    QString moreAboutLink(QLatin1String("<a href=\"%1\"><font color=#333333>%2</font></a>"));
    ui->lLearnMore->setText(moreAboutLink.arg(url,tr("Learn more")));
    ui->lLearnMore->show();
}

void DuplicatedNodeItem::fillUi()
{
    switch(mType)
    {
        case NodeItemType::DONT_UPLOAD:
        {
            ui->bAction->setText(tr("Skip"));
            ui->lTitle->setText(mInfo->isLocalFile() ? tr("Skip this file") : tr("Skip this folder"));
            break;
        }
        case NodeItemType::FOLDER_UPLOAD_AND_MERGE:
        {
            setActionAndTitle(tr("Merge"));
            break;
        }
        case NodeItemType::UPLOAD_AND_RENAME:
        {
            setActionAndTitle(tr("Rename"));
            ui->wNodeInfo->hide();
            break;
        }
        case NodeItemType::FILE_UPLOAD_AND_REPLACE:
        {
            setActionAndTitle(tr("Replace"));
            break;
        }
        case NodeItemType::FILE_UPLOAD_AND_UPDATE:
        {
            setActionAndTitle(tr("Update"));
            break;
        }
    }

    auto nodeName(getNodeName());

    QIcon icon = isFile() ? QIcon(Utilities::getExtensionPixmapName(nodeName, QLatin1Literal(":/images/drag_")))
                                        : QIcon(QLatin1Literal(":/images/icons/folder/medium-folder.png"));

    ui->lIcon->setPixmap(icon.pixmap(ui->lIcon->size()));

    updateSize();
    updateModificationTime();
}

bool DuplicatedNodeItem::isModifiedTimeVisible()
{
    return mModifiedTimeVisible;
}

void DuplicatedNodeItem::setModifiedTimeVisible(bool state)
{
    mModifiedTimeVisible = state;
}

bool DuplicatedNodeItem::isValid() const
{
    return mInfo != nullptr;
}

void DuplicatedNodeItem::setActionAndTitle(const QString &text)
{
    ui->bAction->setText(text);
    ui->lTitle->setText(text);
}

void DuplicatedNodeItem::updateSize()
{
    auto nodeSize(getNodeSize());

    QString nodeSizeText = nodeSize < 0 ? tr("loading size…") : Utilities::getSizeString(nodeSize);
    ui->lSize->setText(nodeSizeText);
}

void DuplicatedNodeItem::updateModificationTime()
{
    auto date = getModifiedTime();

    bool visible = isModifiedTimeVisible();
    if(visible)
    {
        QString dateString;
        if(date.isValid())
        {
            auto format = QLocale::system().dateFormat(QLocale::LongFormat);
            format.remove(QLatin1Char(','));
            format.remove(QLatin1String("dddd"));
            format = format.trimmed();

            dateString = date.toString(format);

            if(mInfo->getNodeModifiedTime().date() == mInfo->getLocalModifiedTime().date())
            {
                QString hour = QString(QLatin1String(", %1")).arg(date.time().toString(QLatin1String("hh:mm")));
                dateString.append(hour);
            }
        }
        else
        {
            dateString = tr("loading time…");
        }

        ui->lDate->setText(dateString);
    }

    ui->lDate->setVisible(visible);
    ui->point->setVisible(visible);
}

bool DuplicatedNodeItem::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lNodeName && event->type() == QEvent::Resize)
    {
        auto nodeName(getNodeName());
        auto elidedName = ui->lDescription->fontMetrics().elidedText(nodeName, Qt::ElideMiddle, ui->lNodeName->width());
        ui->lNodeName->setText(elidedName);

        if (elidedName != nodeName)
        {
            ui->lNodeName->setToolTip(nodeName);
        }
    }

    return QWidget::eventFilter(watched, event);
}

void DuplicatedNodeItem::on_bAction_clicked()
{
    emit actionClicked();
}

void DuplicatedNodeItem::onNodeSizeFinished()
{
    mNodeSize = mFolderSizeFuture.result();
    updateSize();
}

/*
 * REMOTE IMPLEMENTATION CLASS
 * USE TO SHOW THE REMOTE NODE INFO
*/
DuplicatedRemoteItem::DuplicatedRemoteItem(QWidget *parent)
    : DuplicatedNodeItem(parent)
{
    mListener = new mega::QTMegaRequestListener(MegaSyncApp->getMegaApi(), this);
}

DuplicatedRemoteItem::~DuplicatedRemoteItem()
{
    delete mListener;
}

std::shared_ptr<mega::MegaNode> DuplicatedRemoteItem::getNode()
{
    return mInfo->getRemoteConflictNode();
}

QString DuplicatedRemoteItem::getNodeName()
{
    return mInfo->getName();
}

QDateTime DuplicatedRemoteItem::getModifiedTime()
{
    return mInfo->getNodeModifiedTime();
}

qint64 DuplicatedRemoteItem::getNodeSize()
{
    if(mInfo->isRemoteFile())
    {
        auto nodeSize = static_cast<qint64>(mInfo->getRemoteConflictNode()->getSize());
        return std::max(nodeSize, 0LL);
    }
    else
    {
        if(mNodeSize < 0)
        {
            MegaSyncApp->getMegaApi()->getFolderInfo(mInfo->getRemoteConflictNode().get(),mListener);
        }

        return mNodeSize;
    }
}

bool DuplicatedRemoteItem::isFile() const
{
    return mInfo->isRemoteFile();
}

void DuplicatedRemoteItem::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    mega::MegaRequestListener::onRequestFinish(api, request, e);

    if (request->getType() == mega::MegaRequest::TYPE_FOLDER_INFO
            && e->getErrorCode() == mega::MegaError::API_OK)
    {
        auto folderInfo = request->getMegaFolderInfo();
        mNodeSize = folderInfo->getCurrentSize();
        updateSize();
    }
}

/*
 * LOCAL IMPLEMENTATION CLASS
 * USE TO SHOW THE LOCAL NODE INFO
*/
DuplicatedLocalItem::DuplicatedLocalItem(QWidget *parent)
    : DuplicatedNodeItem(parent)
{
    connect(&mFolderModificationTimeFuture, &QFutureWatcher<QDateTime>::finished, this, &DuplicatedLocalItem::onNodeModificationTimeFinished);
}

const QString &DuplicatedLocalItem::getLocalPath()
{
    return mInfo->getLocalPath();
}

QString DuplicatedLocalItem::getNodeName()
{
    if(mInfo->isLocalFile())
    {
        QFileInfo localNode(mInfo->getLocalPath());
        return localNode.fileName();
    }
    else
    {
        QDir dir(mInfo->getLocalPath());
        return dir.dirName();
    }
}

QDateTime DuplicatedLocalItem::getModifiedTime()
{
    if(!mModificationTime.isValid())
    {
        if(mInfo->isLocalFile())
        {
            QFileInfo localNode(mInfo->getLocalPath());
            mModificationTime = localNode.lastModified();
            mInfo->setLocalModifiedTime(mModificationTime);
        }
        //Is local folder
        else
        {
            auto future = QtConcurrent::run([this]() -> QDateTime{
                return getFolderModifiedDate(mInfo->getLocalPath(), mModificationTime);
            });
            mFolderModificationTimeFuture.setFuture(future);
        }
    }

    return mModificationTime;
}

qint64 DuplicatedLocalItem::getNodeSize()
{
    if(mInfo->isLocalFile())
    {
        QFileInfo localNode(mInfo->getLocalPath());
        mNodeSize = localNode.size();
    }
    else
    {
        if(mNodeSize < 0)
        {
            auto future = QtConcurrent::run([this]() -> qint64{
                qint64 size = getFolderSize(mInfo->getLocalPath(), mNodeSize);
                return std::max(size, 0LL);
            });
            mFolderSizeFuture.setFuture(future);
        }
    }

    return mNodeSize;
}

bool DuplicatedLocalItem::isFile() const
{
    return mInfo->isLocalFile();
}

qint64 DuplicatedLocalItem::getFolderSize(const QString &path, qint64 size)
{
    auto newSize(size);

    QDir folder(path);
    folder.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList fileList = folder.entryList();
    for (int i=0; i < fileList.count(); i++)
    {
        QString filePath = getFullFileName(folder.absolutePath(), fileList.at(i));
        QFileInfo fileInfo(filePath);
        newSize += fileInfo.size();
    }

    folder.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = folder.entryList();
    for (int i=0; i < dirList.size(); ++i)
    {
        QString newPath = getFullFileName(folder.absolutePath(), dirList.at(i));
        newSize = getFolderSize(newPath, newSize);
    }

    return newSize;
}

QDateTime DuplicatedLocalItem::getFolderModifiedDate(const QString &path, const QDateTime& date)
{
    QDateTime newDate(date);

    QDir folder(path);
    folder.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList fileList = folder.entryList();

    for (int i=0; i < fileList.count(); i++)
    {
        QString filePath = getFullFileName(folder.absolutePath(), fileList.at(i));
        QFileInfo fileInfo(filePath);

        if(fileInfo.lastModified() > newDate)
        {
            newDate = fileInfo.lastModified();
        }
    }

    folder.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = folder.entryList();
    for (int i=0; i < dirList.size(); ++i)
    {
        QString newPath = getFullFileName(folder.absolutePath(),dirList.at(i));
        newDate = getFolderModifiedDate(newPath, newDate);
    }

    if(!newDate.isValid() && fileList.isEmpty() && dirList.isEmpty())
    {
        setModifiedTimeVisible(false);
    }

    return newDate;
}

QString DuplicatedLocalItem::getFullFileName(const QString &path, const QString &fileName)
{
    return QString(QString::fromUtf8("%1/%2")).arg(path, fileName);
}

void DuplicatedLocalItem::onNodeModificationTimeFinished()
{
    mModificationTime = mFolderModificationTimeFuture.result();
    mInfo->setLocalModifiedTime(mModificationTime);
}

/*
 * RENAME IMPLEMENTATION FOR LOCAL NODES
 * USE TO SHOW THE RENAME OPTION
*/
DuplicatedRenameItem::DuplicatedRenameItem(QWidget *parent)
    :DuplicatedLocalItem(parent)
{
    setModifiedTimeVisible(false);
}

void DuplicatedRenameItem::setInfo(std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    DuplicatedLocalItem::setInfo(conflict, NodeItemType::UPLOAD_AND_RENAME);
}

QString DuplicatedRenameItem::getNodeName()
{
    return mInfo->getNewName();
}

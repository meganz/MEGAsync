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
    ui(new Ui::DuplicatedNodeItem)
{
    ui->setupUi(this);
    ui->lNodeName->installEventFilter(this);
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
                                        : QIcon(QLatin1Literal(":/images/color_folder@2x.png"));

    ui->lIcon->setPixmap(icon.pixmap(ui->lIcon->size()));

    updateSize();
    updateModificationTime();
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

QString DuplicatedNodeItem::getFilesAndFolders(int folders, int files)
{
    QString foldersText(tr("%1 folder", "", folders).arg(folders));
    QString filesText(tr("%1 file", "", files).arg(files));

    QString folderContains;

    if(!foldersText.isEmpty() && !filesText.isEmpty())
    {
        folderContains = QString::fromLatin1("%1·%2").arg(foldersText, filesText);
    }
    else if(!foldersText.isEmpty())
    {
        folderContains = foldersText;
    }
    else if(!filesText.isEmpty())
    {
        folderContains = filesText;
    }

    return folderContains;
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
    }

    return QWidget::eventFilter(watched, event);
}

void DuplicatedNodeItem::on_bAction_clicked()
{
    emit actionClicked();
}

/*
 * REMOTE IMPLEMENTATION CLASS
 * USE TO SHOW THE REMOTE NODE INFO
*/
DuplicatedRemoteItem::DuplicatedRemoteItem(QWidget *parent)
    : DuplicatedNodeItem(parent)
{}

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
    auto nodeSize = mInfo->getRemoteConflictNode()->getSize();
    return std::max(nodeSize, 0L);
}

bool DuplicatedRemoteItem::isFile() const
{
    return mInfo->isRemoteFile();
}

bool DuplicatedRemoteItem::isModifiedTimeVisible() const
{
    return true;
}

/*
 * LOCAL IMPLEMENTATION CLASS
 * USE TO SHOW THE LOCAL NODE INFO
*/
DuplicatedLocalItem::DuplicatedLocalItem(QWidget *parent)
    : DuplicatedNodeItem(parent), mNodeSize(-1),mValidModificationTime(false)
{
    connect(&mFolderSizeFuture, &QFutureWatcher<void>::finished, this, &DuplicatedLocalItem::onNodeSizeFinished);
    connect(&mFolderModificationTimeFuture, &QFutureWatcher<void>::finished, this, &DuplicatedLocalItem::onNodeModificationTimeFinished);
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
    if(mModificationTime.isValid())
    {
        return mModificationTime;
    }

    if(mInfo->isLocalFile())
    {
        QFileInfo localNode(mInfo->getLocalPath());
        mModificationTime = localNode.lastModified();
        onNodeModificationTimeFinished();
        return mModificationTime;
    }
    else
    {
        auto future = QtConcurrent::run([this](){
            getFolderModifiedDate(mInfo->getLocalPath());
        });
        mFolderModificationTimeFuture.setFuture(future);
        return QDateTime();
    }

}

bool DuplicatedLocalItem::isModifiedTimeVisible() const
{
    return mValidModificationTime;
}

qint64 DuplicatedLocalItem::getNodeSize()
{
    if(mInfo->isLocalFile())
    {
        QFileInfo localNode(mInfo->getLocalPath());
        return localNode.size();
    }
    else
    {
        if(mNodeSize < 0)
        {
            auto future = QtConcurrent::run([this](){
                getFolderSize(mInfo->getLocalPath());
            });
            mFolderSizeFuture.setFuture(future);
            return mNodeSize;
        }
        else
        {
            return mNodeSize;
        }
    }
}

bool DuplicatedLocalItem::isFile() const
{
    return mInfo->isLocalFile();
}

void DuplicatedLocalItem::getFolderSize(const QString &path)
{
    QDir folder(path);
    folder.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList fileList = folder.entryList();
    for (int i=0; i < fileList.count(); i++)
    {
        QString filePath = getFullFileName(folder.absolutePath(), fileList.at(i));
        QFileInfo fileInfo(filePath);
        mNodeSize += fileInfo.size();
    }

    folder.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = folder.entryList();
    for (int i=0; i < dirList.size(); ++i)
    {
        QString newPath = getFullFileName(folder.absolutePath(), dirList.at(i));
        getFolderSize(newPath);
    }
}

void DuplicatedLocalItem::getFolderModifiedDate(const QString &path)
{
    QDir folder(path);
    folder.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList fileList = folder.entryList();

    for (int i=0; i < fileList.count(); i++)
    {
        QString filePath = getFullFileName(folder.absolutePath(), fileList.at(i));
        QFileInfo fileInfo(filePath);
        if(fileInfo.lastModified() > mModificationTime)
        {
            mModificationTime = fileInfo.lastModified();
        }
    }

    folder.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = folder.entryList();
    for (int i=0; i < dirList.size(); ++i)
    {
        QString newPath = getFullFileName(folder.absolutePath(),dirList.at(i));
        getFolderModifiedDate(newPath);
    }
}

QString DuplicatedLocalItem::getFullFileName(const QString &path, const QString &fileName)
{
    return QString(QString::fromUtf8("%1/%2")).arg(path, fileName);
}

void DuplicatedLocalItem::onNodeSizeFinished()
{
    updateSize();
}

void DuplicatedLocalItem::onNodeModificationTimeFinished()
{
    mValidModificationTime = mModificationTime.isValid();
    mInfo->setLocalModifiedTime(mModificationTime);
}

/*
 * RENAME IMPLEMENTATION FOR LOCAL NODES
 * USE TO SHOW THE RENAME OPTION
*/
DuplicatedRenameItem::DuplicatedRenameItem(QWidget *parent)
    :DuplicatedLocalItem(parent)
{
}

void DuplicatedRenameItem::setInfo(std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    DuplicatedLocalItem::setInfo(conflict, NodeItemType::UPLOAD_AND_RENAME);
}

QString DuplicatedRenameItem::getNodeName()
{
    return mInfo->getNewName();
}

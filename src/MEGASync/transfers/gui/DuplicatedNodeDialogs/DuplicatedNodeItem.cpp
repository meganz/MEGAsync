#include "DuplicatedNodeItem.h"
#include "ui_DuplicatedNodeItem.h"

#include <Utilities.h>
#include <MegaApplication.h>

#include "mega/types.h"

/*
 * BASE CLASS
*/
DuplicatedNodeItem::DuplicatedNodeItem(QWidget *parent) :
    QWidget(parent),
    mInfo(nullptr),
    mNodeSize(-1),
    ui(new Ui::DuplicatedNodeItem)
{
    ui->setupUi(this);
    ui->lLearnMore->hide();
}

DuplicatedNodeItem::~DuplicatedNodeItem()
{
    delete ui;
}

void DuplicatedNodeItem::setInfo(std::shared_ptr<DuplicatedNodeInfo>info, NodeItemType type)
{
    mInfo = info;
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
    ui->lNodeName->setText(nodeName);

    QIcon icon = isFile() ? QIcon(Utilities::getExtensionPixmapName(nodeName, QLatin1Literal(":/images/drag_")))
                                        : QIcon(QLatin1Literal(":/images/icons/folder/medium-folder.png"));

    ui->lIcon->setPixmap(icon.pixmap(ui->lIcon->size()));
}

void DuplicatedNodeItem::setModifiedTime(const QDateTime& dateTime)
{
    auto timeString = dateTime.isValid() ? MegaSyncApp->getFormattedDateByCurrentLanguage(dateTime, QLocale::FormatType::ShortFormat) : tr("loading time…");
    ui->lDate->setText(timeString);
}

void DuplicatedNodeItem::setSize(qint64 size)
{
    QString nodeSizeText(size < 0 ? tr("loading size…") : Utilities::getSizeString(size));
    ui->lSize->setText(nodeSizeText);
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
{
}

DuplicatedRemoteItem::~DuplicatedRemoteItem()
{
    mFolderAttributes->cancel();
}

void DuplicatedRemoteItem::setInfo(std::shared_ptr<DuplicatedNodeInfo> info, NodeItemType type)
{
    if(!mFolderAttributes)
    {
        mFolderAttributes = new RemoteFileFolderAttributes(info->getRemoteConflictNode()->getHandle(), this, false);
        mFolderAttributes->requestModifiedTime(this,[this](const QDateTime& time)
        {
            setModifiedTime(time);
        });
        mFolderAttributes->requestSize(this,[this](qint64 size)
        {
            setSize(size);
        });
    }

    DuplicatedNodeItem::setInfo(info, type);
}

std::shared_ptr<mega::MegaNode> DuplicatedRemoteItem::getNode()
{
    return mInfo->getRemoteConflictNode();
}

QString DuplicatedRemoteItem::getNodeName()
{
    return mInfo->getName();
}

bool DuplicatedRemoteItem::isFile() const
{
    return mInfo->isRemoteFile();
}

/*
 * LOCAL IMPLEMENTATION CLASS
 * USE TO SHOW THE LOCAL NODE INFO
*/
DuplicatedLocalItem::DuplicatedLocalItem(QWidget *parent)
    : DuplicatedNodeItem(parent)
{
}

DuplicatedLocalItem::~DuplicatedLocalItem()
{
    mFolderAttributes->cancel();
}

void DuplicatedLocalItem::setInfo(std::shared_ptr<DuplicatedNodeInfo> info, NodeItemType type)
{
    if(!mFolderAttributes)
    {
        mFolderAttributes = new LocalFileFolderAttributes(info->getLocalPath(), this);
        mFolderAttributes->requestModifiedTime(this,[this](const QDateTime& time)
        {
            setModifiedTime(time);
        });
        mFolderAttributes->requestSize(this,[this](qint64 size)
        {
            setSize(size);
        });
    }

    DuplicatedNodeItem::setInfo(info, type);
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

bool DuplicatedLocalItem::isFile() const
{
    return mInfo->isLocalFile();
}

QString DuplicatedLocalItem::getFullFileName(const QString &path, const QString &fileName)
{
    return QString(QString::fromUtf8("%1/%2")).arg(path, fileName);
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
    return mInfo->getDisplayNewName();
}

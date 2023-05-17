#include "FolderAttributes.h"

#include <MegaApplication.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

FolderAttributes::FolderAttributes(QObject* parent)
    : mSize(-1),
      mCancelled(false),
      QObject(parent)
{
}

FolderAttributes::~FolderAttributes()
{
}

void FolderAttributes::cancel()
{
    mCancelled = true;
}


//LOCAL
LocalFolderAttributes::LocalFolderAttributes(const QString &path, QObject *parent)
    : mPath(path),
      FolderAttributes(parent)
{
    connect(&mModifiedTimeWatcher, &QFutureWatcher<QDateTime>::finished,
            this, &LocalFolderAttributes::onModifiedTimeCalculated);

    connect(&mFolderSizeFuture, &QFutureWatcher<qint64>::finished,
            this, &LocalFolderAttributes::onSizeCalculated);

    QDirIterator filesIt(mPath, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    mIsEmpty = !filesIt.hasNext();
}

void LocalFolderAttributes::requestSize()
{
    QFileInfo fileInfo(mPath);

    if(fileInfo.isFile())
    {
        mSize = fileInfo.size();
        emit sizeReady(mSize);
    }
    else
    {
        if(mSize < 0)
        {
            auto future = QtConcurrent::run([this]() -> qint64{
                return calculateSize();
            });
            mFolderSizeFuture.setFuture(future);
        }
    }
}

void LocalFolderAttributes::requestModifiedTime()
{
    QFileInfo fileInfo(mPath);

    if(fileInfo.isFile())
    {
        mModifiedTime = fileInfo.lastModified();
        emit modifiedTimeReady(mModifiedTime);
    }
    //Is local folder
    else
    {
        if(mIsEmpty)
        {
            requestCreatedTime();
        }
        else
        {
            auto future = QtConcurrent::run([this]() -> QDateTime{
                return calculateModifiedTime();
            });
            mModifiedTimeWatcher.setFuture(future);
        }
    }
}

void LocalFolderAttributes::onModifiedTimeCalculated()
{
    mModifiedTime = mModifiedTimeWatcher.result();
    if(mModifiedTime.isValid())
    {
        emit modifiedTimeReady(mModifiedTime);
    }
}

void LocalFolderAttributes::onSizeCalculated()
{
    mSize = mFolderSizeFuture.result();
    emit sizeReady(mSize);
}

void LocalFolderAttributes::requestCreatedTime()
{
    struct stat result;
    if(stat(mPath.toUtf8().constData(), &result)==0)
    {
        auto mod_time = result.st_ctime;
        mCreatedTime = QDateTime::fromSecsSinceEpoch(mod_time);
        emit createdTimeReady(mCreatedTime);

        QFileInfo fileInfo(mPath);
        if(!fileInfo.isFile())
        {
            if(mIsEmpty)
            {
               mModifiedTime = mCreatedTime;
               emit modifiedTimeReady(mModifiedTime);
            }
        }
    }
}

QDateTime LocalFolderAttributes::calculateModifiedTime()
{
    QDateTime newDate;
    QDirIterator filesIt(mPath, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while (filesIt.hasNext())
    {
        if(mCancelled)
        {
            break;
        }

        filesIt.next();
        if(filesIt.fileInfo().lastModified() > newDate)
        {
            newDate = filesIt.fileInfo().lastModified();
        }
    }

    return newDate;
}

qint64 LocalFolderAttributes::calculateSize()
{
    qint64 newSize(0);

    QDirIterator filesIt(mPath, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while (filesIt.hasNext())
    {
        filesIt.next();
        newSize += filesIt.fileInfo().size();
    }

    return newSize;
}

//REMOTE
RemoteFolderAttributes::RemoteFolderAttributes(mega::MegaHandle handle, QObject* parent)
    : mHandle(handle),
      FolderAttributes(parent)
{
    mListener = new mega::QTMegaRequestListener(MegaSyncApp->getMegaApi(), this);
}

RemoteFolderAttributes::~RemoteFolderAttributes()
{
    delete mListener;
}

void RemoteFolderAttributes::requestSize()
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    if(node)
    {
        if(node->isFile())
        {
            mSize = std::max(node->getSize(), 0LL);
            emit sizeReady(mSize);
        }
        else
        {
            if(mSize < 0)
            {
                MegaSyncApp->getMegaApi()->getFolderInfo(node.get(),mListener);
            }
        }
    }
}

void RemoteFolderAttributes::requestModifiedTime()
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    if(node)
    {
        auto time = node->isFile() ? node->getModificationTime()
                                                    : node->getCreationTime();
        mModifiedTime = QDateTime::fromSecsSinceEpoch(time);
        emit modifiedTimeReady(mModifiedTime);
    }
}

void RemoteFolderAttributes::requestCreatedTime()
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    if(node)
    {
        auto time = node->isFile() ? node->getCreationTime()
                                                    : node->getCreationTime();
        mCreatedTime = QDateTime::fromSecsSinceEpoch(time);
        emit modifiedTimeReady(mCreatedTime);
    }
}

void RemoteFolderAttributes::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    mega::MegaRequestListener::onRequestFinish(api, request, e);

    if (request->getType() == mega::MegaRequest::TYPE_FOLDER_INFO
            && e->getErrorCode() == mega::MegaError::API_OK)
    {
        auto folderInfo = request->getMegaFolderInfo();
        mSize = std::max(folderInfo->getCurrentSize(), 0LL);
        emit sizeReady(mSize);
    }
}

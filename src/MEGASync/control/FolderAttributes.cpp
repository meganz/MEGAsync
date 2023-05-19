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

FileFolderAttributes::FileFolderAttributes(QObject* parent)
    : mCancelled(false),
      mSize(-1),
      QObject(parent)
{
}

FileFolderAttributes::~FileFolderAttributes()
{
}

void FileFolderAttributes::requestSize(QObject* caller,std::function<void (qint64)> func)
{
    connect(this, &FileFolderAttributes::sizeReady, caller, [func](qint64 size){
        if(func)
        {
            func(size);
        }
    });
}

void FileFolderAttributes::requestModifiedTime(QObject* caller, std::function<void (const QDateTime &)> func)
{
    connect(this, &FileFolderAttributes::modifiedTimeReady, caller, [func](const QDateTime& time){
        if(func)
        {
            func(time);
        }
    });
}

void FileFolderAttributes::requestCreatedTime(QObject* caller, std::function<void (const QDateTime &)> func)
{
    connect(this, &FileFolderAttributes::createdTimeReady, caller, [func](const QDateTime& time){
        if(func)
        {
            func(time);
        }
    });
}

void FileFolderAttributes::cancel()
{
    mCancelled = true;
}


//LOCAL
LocalFileFolderAttributes::LocalFileFolderAttributes(const QString &path, QObject *parent)
    : mPath(path),
      FileFolderAttributes(parent)
{
    connect(&mModifiedTimeWatcher, &QFutureWatcher<QDateTime>::finished,
            this, &LocalFileFolderAttributes::onModifiedTimeCalculated);

    connect(&mFolderSizeFuture, &QFutureWatcher<qint64>::finished,
            this, &LocalFileFolderAttributes::onSizeCalculated);

    QDirIterator filesIt(mPath, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    mIsEmpty = !filesIt.hasNext();
}

void LocalFileFolderAttributes::requestSize(QObject* caller,std::function<void(qint64)> func)
{
    FileFolderAttributes::requestSize(caller,func);

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
        else
        {
            emit sizeReady(mSize);
        }
    }
}

void LocalFileFolderAttributes::requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestModifiedTime(caller,func);

    QFileInfo fileInfo(mPath);

    if(fileInfo.exists())
    {
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
                requestCreatedTime(caller,func);
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
    else if(mModifiedTime.isValid())
    {
        emit modifiedTimeReady(mModifiedTime);
    }
}

void LocalFileFolderAttributes::onModifiedTimeCalculated()
{
    mModifiedTime = mModifiedTimeWatcher.result();
    if(mModifiedTime.isValid())
    {
        emit modifiedTimeReady(mModifiedTime);
    }
}

void LocalFileFolderAttributes::onSizeCalculated()
{
    mSize = mFolderSizeFuture.result();
    emit sizeReady(mSize);
}

void LocalFileFolderAttributes::requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestCreatedTime(caller,func);

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
    else if(mCreatedTime.isValid())
    {
        emit createdTimeReady(mCreatedTime);
    }
}

QDateTime LocalFileFolderAttributes::calculateModifiedTime()
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

qint64 LocalFileFolderAttributes::calculateSize()
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
RemoteFileFolderAttributes::RemoteFileFolderAttributes(mega::MegaHandle handle, QObject* parent)
    : mHandle(handle),
      FileFolderAttributes(parent)
{
    mListener = new mega::QTMegaRequestListener(MegaSyncApp->getMegaApi(), this);
}

RemoteFileFolderAttributes::~RemoteFileFolderAttributes()
{
    delete mListener;
}

void RemoteFileFolderAttributes::requestSize(QObject* caller,std::function<void(qint64)> func)
{
    FileFolderAttributes::requestSize(caller,func);

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    if(node)
    {
        if(node->isFile())
        {
            mSize = std::max(static_cast<long long>(node->getSize()), static_cast<long long>(0));
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

void RemoteFileFolderAttributes::requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestModifiedTime(caller,func);

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    if(node)
    {
        auto time = node->isFile() ? node->getModificationTime()
                                                    : node->getCreationTime();
        mModifiedTime = QDateTime::fromSecsSinceEpoch(time);
        emit modifiedTimeReady(mModifiedTime);
    }
}

void RemoteFileFolderAttributes::requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestCreatedTime(caller, func);

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    if(node)
    {
        auto time = node->isFile() ? node->getCreationTime()
                                                    : node->getCreationTime();
        mCreatedTime = QDateTime::fromSecsSinceEpoch(time);
        emit modifiedTimeReady(mCreatedTime);
    }
}

void RemoteFileFolderAttributes::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
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

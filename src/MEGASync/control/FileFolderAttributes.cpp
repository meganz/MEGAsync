#include "FileFolderAttributes.h"

#include <MegaApplication.h>
#include <UserAttributesRequests/FullName.h>

#include <QEventLoop>

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

void FileFolderAttributes::initAllAttributes()
{
    requestSize(nullptr, nullptr);
    requestModifiedTime(nullptr, nullptr);
    requestCreatedTime(nullptr, nullptr);
    requestCRC(nullptr, nullptr);
}

void FileFolderAttributes::requestSize(QObject* caller,std::function<void (qint64)> func)
{
    if(auto context = requestReady(AttributeTypes::Size, caller))
    {
        connect(this, &FileFolderAttributes::sizeReady, context, [this, func](qint64 size){
            if(func)
            {
                func(size);
                if(size >= 0)
                {
                    requestFinish(AttributeTypes::Size);
                }
            }
        });
    }
}

void FileFolderAttributes::requestModifiedTime(QObject* caller, std::function<void (const QDateTime &)> func)
{
    if(auto context = requestReady(AttributeTypes::ModifiedTime, caller))
    {
        connect(this, &FileFolderAttributes::modifiedTimeReady, context, [this, func](const QDateTime& time){
            if(func)
            {
                func(time);
                if(time.isValid())
                {
                    requestFinish(AttributeTypes::ModifiedTime);
                }
            }
        });
    }
}

void FileFolderAttributes::requestCreatedTime(QObject* caller, std::function<void (const QDateTime &)> func)
{
    if(auto context = requestReady(AttributeTypes::CreatedTime, caller))
    {
        connect(this, &FileFolderAttributes::createdTimeReady, context, [this, func](const QDateTime& time){
            if(func)
            {
                func(time);
                if(time.isValid())
                {
                    requestFinish(AttributeTypes::CreatedTime);
                }
            }
        });
    }
}

void FileFolderAttributes::requestCRC(QObject *caller, std::function<void (const QString &)> func)
{
    if(auto context = requestReady(AttributeTypes::CRC, caller))
    {
        connect(this, &FileFolderAttributes::CRCReady, context, [this, func](const QString& fp){
            if(func)
            {
                func(fp);
                requestFinish(AttributeTypes::CRC);
            }
        });
    }
}

void FileFolderAttributes::cancel()
{
    mCancelled = true;
}

int64_t FileFolderAttributes::size() const
{
    return mSize;
}

int64_t FileFolderAttributes::modifiedTime() const
{
    return mModifiedTime.toSecsSinceEpoch();
}

int64_t FileFolderAttributes::createdTime() const
{
    return mCreatedTime.toSecsSinceEpoch();
}

const QString &FileFolderAttributes::fingerprint() const
{
    return mFp;
}

bool FileFolderAttributes::attributeNeedsUpdate(int type)
{
    auto currentTime = QDateTime::currentMSecsSinceEpoch();
    if(!mRequestTimestamps.contains(type)
            || ((currentTime - mRequestTimestamps.value(type)) > 2000))
    {
        mRequestTimestamps.insert(type, currentTime);
        return true;
    }

    return false;
}

void FileFolderAttributes::requestFinish(int type)
{
    auto contextObject = mRequests.take(type);
    if(contextObject)
    {
        contextObject->deleteLater();
    }
}

QObject *FileFolderAttributes::requestReady(int type, QObject *caller)
{
    if(caller && !mRequests.contains(type))
    {
        QObject* contextObject = new QObject(caller);
        mRequests.insert(type, contextObject);

        return contextObject;
    }

    return nullptr;
}


//LOCAL
LocalFileFolderAttributes::LocalFileFolderAttributes(const QString &path, QObject *parent)
    : FileFolderAttributes(parent),
      mPath(path)
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

    if(!mPath.isEmpty() && attributeNeedsUpdate(AttributeTypes::Size))
    {
        QFileInfo fileInfo(mPath);
        if(fileInfo.exists())
        {
            if(fileInfo.isFile())
            {
                mSize = fileInfo.size();
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
    }

    //We always send the size, even if the request is async...just to show on GUI a "loading size..." or the most recent size while the new is received
    emit sizeReady(mSize);
}

void LocalFileFolderAttributes::requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestModifiedTime(caller,func);

    if(!mPath.isEmpty() && attributeNeedsUpdate(AttributeTypes::ModifiedTime))
    {
        QFileInfo fileInfo(mPath);

        if(fileInfo.exists())
        {
            if(fileInfo.isFile())
            {
                mModifiedTime = fileInfo.lastModified();
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
    }

     //We always send the time, even if the request is async...just to show on GUI a "loading time..." or the most recent time while the new is received
    emit modifiedTimeReady(mModifiedTime);
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
    //Created time not available for LINUX
    FileFolderAttributes::requestCreatedTime(caller,func);

    if(!mPath.isEmpty() && attributeNeedsUpdate(AttributeTypes::CreatedTime))
    {
        QFileInfo fileInfo(mPath);
        if(fileInfo.exists())
        {
#ifdef Q_OS_WINDOWS
            struct stat result;
            const QString sourcePath = mPath;
            QVarLengthArray<wchar_t, MAX_PATH + 1> file(sourcePath.length() + 2);
            sourcePath.toWCharArray(file.data());
            file[sourcePath.length()] = wchar_t{};
            file[sourcePath.length() + 1] = wchar_t{};
            if(_wstat(file.constData(), &result)==0)
            {
                mCreatedTime = QDateTime::fromSecsSinceEpoch(result.st_ctime);
            }
#elif defined(Q_OS_MACOS)
            struct stat the_time;
            stat(mPath.toUtf8(), &the_time);
            mCreatedTime.setTime_t(the_time.st_birthtimespec.tv_sec);
#endif
            if(!fileInfo.isFile())
            {
                if(mIsEmpty)
                {
                    mModifiedTime = mCreatedTime;
                    emit modifiedTimeReady(mModifiedTime);
                }
            }
            emit createdTimeReady(mCreatedTime);
        }
    }
}

void LocalFileFolderAttributes::requestCRC(QObject *caller, std::function<void (const QString &)> func)
{
    FileFolderAttributes::requestCRC(caller,func);

    if(!mPath.isEmpty())
    {
        QFileInfo fileInfo(mPath);

        if(fileInfo.exists())
        {
            if(fileInfo.isFile())
            {
                std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRC(QDir::toNativeSeparators(fileInfo.filePath()).toUtf8().constData()));
                mFp = QString::fromUtf8(crc.get());
            }

        }

        emit CRCReady(mFp);
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

    QFileInfo fileInfo(mPath);
    if(!mPath.isEmpty() && fileInfo.exists())
    {
        QDirIterator filesIt(mPath, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);

        while (filesIt.hasNext())
        {
            filesIt.next();
            newSize += filesIt.fileInfo().size();
        }
    }

    return newSize;
}

void LocalFileFolderAttributes::setPath(const QString &newPath)
{
    mPath = newPath;
}

//REMOTE
RemoteFileFolderAttributes::RemoteFileFolderAttributes(const QString &filePath, QObject *parent, bool waitForAttributes)
    : FileFolderAttributes(parent),
      mFilePath(filePath),
      mHandle(mega::INVALID_HANDLE),
      mVersionCount(0),
      mFileCount(-1),
      mWaitForAttributes(waitForAttributes)

{
}

RemoteFileFolderAttributes::RemoteFileFolderAttributes(mega::MegaHandle handle, QObject* parent, bool waitForAttributes)
    : FileFolderAttributes(parent),
      mHandle(handle),
      mVersionCount(0),
      mFileCount(-1),
      mWaitForAttributes(waitForAttributes)
{
}

RemoteFileFolderAttributes::~RemoteFileFolderAttributes()
{
}

void RemoteFileFolderAttributes::initAllAttributes()
{
    FileFolderAttributes::initAllAttributes();

    requestUser(nullptr, nullptr);
    requestVersions(nullptr, nullptr);
    requestFileCount(nullptr, nullptr);
}

void RemoteFileFolderAttributes::requestSize(QObject* caller,std::function<void(qint64)> func)
{
    FileFolderAttributes::requestSize(caller,func);

    if(attributeNeedsUpdate(AttributeTypes::Size))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if(node)
        {
            if(node->isFile())
            {
                mSize = std::max(static_cast<long long>(node->getSize()), static_cast<long long>(0));
            }
            else
            {
                MegaSyncApp->getMegaApi()->getFolderInfo(node.get(), new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                              [this]
                                                                                              (bool, const mega::MegaRequest& request, const mega::MegaError& e)
                {
                    if (request.getType() == mega::MegaRequest::TYPE_FOLDER_INFO
                        && e.getErrorCode() == mega::MegaError::API_OK)
                    {
                        auto folderInfo = request.getMegaFolderInfo();
                        if(folderInfo)
                        {
                            mSize = std::max(folderInfo->getCurrentSize(), 0LL);
                            emit sizeReady(mSize);
                        }
                    }

                    if(mEventLoop.isRunning())
                    {
                        mEventLoop.quit();
                    }
                }));

                if(mWaitForAttributes && mSize < 0 && !mEventLoop.isRunning())
                {
                    mEventLoop.exec();
                }
            }
        }
    }

    //We always send the size, even if the request is async...just to show on GUI a "loading size..." or the most recent size while the new is received
    emit sizeReady(mSize);
}

void RemoteFileFolderAttributes::requestFileCount(QObject *caller, std::function<void (int)> func)
{
    std::unique_ptr<mega::MegaNode> node = getNode();
    if(node && node->isFolder())
    {
        FileFolderAttributes::requestCustomValue<RemoteFileFolderAttributes, int>(this, caller, func, &RemoteFileFolderAttributes::fileCountReady,
                                                                                  RemoteFileFolderAttributes::FileCount);

        if(attributeNeedsUpdate(RemoteFileFolderAttributes::FileCount))
        {
            MegaSyncApp->getMegaApi()->getFolderInfo(node.get(),new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                          [this, func]
                                                                                          (bool, const mega::MegaRequest& request, const mega::MegaError& e)
            {
                if (request.getType() == mega::MegaRequest::TYPE_FOLDER_INFO
                        && e.getErrorCode() == mega::MegaError::API_OK)
                {
                    auto folderInfo = request.getMegaFolderInfo();
                    if(folderInfo)
                    {
                        mFileCount = std::max(folderInfo->getNumFiles(), 0);
                    }
                }

                if(mEventLoop.isRunning())
                {
                    mEventLoop.quit();
                }
            }));
            if(mWaitForAttributes && mFileCount < 0 && !mEventLoop.isRunning())
            {
                mEventLoop.exec();
            }

            //We always send the size, even if the request is async...just to show on GUI a "loading size..." or the most recent size while the new is received
            emit fileCountReady(mFileCount);
        }
    }
}

void RemoteFileFolderAttributes::requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestModifiedTime(caller,func);

    if(attributeNeedsUpdate(AttributeTypes::ModifiedTime))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if(node)
        {
            int64_t newTime = node->isFile() ? node->getModificationTime()
                                     : node->getCreationTime();

            mModifiedTime = QDateTime::fromSecsSinceEpoch(newTime);
        }
    }

    emit modifiedTimeReady(mModifiedTime);
}

void RemoteFileFolderAttributes::requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    FileFolderAttributes::requestCreatedTime(caller, func);

    if(!mCreatedTime.isValid() && attributeNeedsUpdate(AttributeTypes::CreatedTime))
    {
        std::unique_ptr<mega::MegaNode> node = getNode(Version::First);
        if(node)
        {
            mCreatedTime = QDateTime::fromSecsSinceEpoch(node->getCreationTime());
        }
    }

    emit createdTimeReady(mCreatedTime);
}

void RemoteFileFolderAttributes::requestCRC(QObject *caller, std::function<void (const QString &)> func)
{
    FileFolderAttributes::requestCRC(caller, func);

    std::unique_ptr<mega::MegaNode> node = getNode();
    if(node)
    {
        if (const char* fp = node->getFingerprint())
        {
            std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRCFromFingerprint(fp));
            mFp = QString::fromUtf8(crc.get());
        }
        else
        {
            mFp = QString();
        }
    }

    emit CRCReady(mFp);
}

void RemoteFileFolderAttributes::requestUser(QObject *caller, std::function<void (QString, bool)> func)
{
    if(attributeNeedsUpdate(RemoteAttributeTypes::User))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if(node)
        {
            auto user = node->getOwner();

            if(user != mega::INVALID_HANDLE && user != mOwner)
            {
                if(auto context = requestReady(RemoteAttributeTypes::User, caller))
                {
                    mOwner = user;
                    MegaSyncApp->getMegaApi()->getUserEmail(user,new mega::OnFinishOneShot(MegaSyncApp->getMegaApi(), [this ,func, context](
                                                                                           bool,
                                                                                           const mega::MegaRequest& request,
                                                                                           const mega::MegaError& e) {
                        if (e.getErrorCode() == mega::MegaError::API_OK)
                        {
                            auto emailFromRequest = request.getEmail();
                            if (emailFromRequest)
                            {
                                mUserEmail = QString::fromUtf8(emailFromRequest);
                                mUserFullName = UserAttributes::FullName::requestFullName(emailFromRequest);

                                if(mUserFullName->isAttributeReady())
                                {
                                    requestFinish(RemoteAttributeTypes::User);
                                }
                                else if(func)
                                {
                                    this->connect(mUserFullName.get(), &UserAttributes::FullName::fullNameReady, context, [this, func]{
                                        func(mUserFullName->getFullName(), true);
                                        requestFinish(RemoteAttributeTypes::User);
                                    });
                                }
                            }
                        }
                    }));
                }
            }
        }
    }

    if(func)
    {
        if(mUserFullName)
        {
            //We always send the name, even if the request is async...just to show on GUI a "loading user..." or the most recent user while the new is received
            func(mUserFullName->getFullName(), true);
        }
        else
        {
            func(QString(), true);
        }
    }
}

void RemoteFileFolderAttributes::requestUser(QObject *caller, mega::MegaHandle currentUser, std::function<void (QString, bool)> func)
{
    std::unique_ptr<mega::MegaNode> node = getNode();
    if(node)
    {
        auto user = node->getOwner();

        if(user != mega::INVALID_HANDLE && user != currentUser)
        {
            requestUser(caller, func);
        }
        else
        {
            mOwner = mega::INVALID_HANDLE;
            mUserEmail.clear();
            mUserFullName = nullptr;
            if(func)
            {
                func(QString(), false);
            }
        }
    }
}

void RemoteFileFolderAttributes::requestVersions(QObject*, std::function<void (int)> func)
{
    if(attributeNeedsUpdate(RemoteFileFolderAttributes::Versions))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if(node)
        {
            mVersionCount = MegaSyncApp->getMegaApi()->getVersions(node.get())->size();
        }
    }

    if(func)
    {
        func(mVersionCount);
    }
}

int RemoteFileFolderAttributes::versionCount()
{
    return mVersionCount;
}

int RemoteFileFolderAttributes::fileCount()
{
    return mFileCount;
}

std::unique_ptr<mega::MegaNode> RemoteFileFolderAttributes::getNode(Version type) const
{
    std::unique_ptr<mega::MegaNode> node;
    std::unique_ptr<mega::MegaNode> lastVersionNode;

    if(mHandle != mega::INVALID_HANDLE)
    {
        node.reset(MegaSyncApp->getMegaApi()->getNodeByHandle(mHandle));
    }

    if(!node)
    {
        node.reset(MegaSyncApp->getMegaApi()->getNodeByPath(mFilePath.toUtf8().constData()));
    }

    if(node)
    {
        std::unique_ptr<mega::MegaNodeList> versions(MegaSyncApp->getMegaApi()->getVersions(node.get()));

        if(versions->size() > 1)
        {
            lastVersionNode.reset(versions->get(type == Version::Last ? 0 : (versions->size() - 1))->copy());
        }
        else
        {
            lastVersionNode = std::move(node);
        }
    }

    return lastVersionNode;
}

void RemoteFileFolderAttributes::setHandle(mega::MegaHandle newHandle)
{
    mHandle = newHandle;
}

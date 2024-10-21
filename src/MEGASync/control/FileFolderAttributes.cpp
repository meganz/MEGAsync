#include "FileFolderAttributes.h"

#include <MegaApplication.h>
#include "FullName.h"

#include <QEventLoop>
#include "RequestListenerManager.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif


#ifdef WIN32
#define stat _stat
#endif

FileFolderAttributes::FileFolderAttributes(QObject* parent)
    : QObject(parent),
    mCancelled(false),
    mValueIsConstant(false)
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

void FileFolderAttributes::setValueUpdatesDisable()
{
    mValueIsConstant = true;
}

bool FileFolderAttributes::areValueUpdatesDisabled() const
{
    return mValueIsConstant;
}


void FileFolderAttributes::cancel()
{
    mCancelled = true;
}

int64_t FileFolderAttributes::size() const
{
    return mValues.value(AttributeTypes::SIZE, -1).value<qint64>();
}

int64_t FileFolderAttributes::modifiedTimeInSecs() const
{
    return mValues.value(AttributeTypes::MODIFIED_TIME).value<QDateTime>().toSecsSinceEpoch();
}

QDateTime FileFolderAttributes::modifiedTime() const
{
    return mValues[AttributeTypes::MODIFIED_TIME].value<QDateTime>();
}

int64_t FileFolderAttributes::createdTimeInSecs() const
{
    return mValues.value(AttributeTypes::CREATED_TIME).value<QDateTime>().toSecsSinceEpoch();
}

QDateTime FileFolderAttributes::createdTime() const
{
    return mValues[AttributeTypes::CREATED_TIME].value<QDateTime>();
}

QString FileFolderAttributes::getCRC() const
{
    return mValues.value(AttributeTypes::CRC).toString();
}

bool FileFolderAttributes::attributeNeedsUpdate(QObject* caller, int type)
{
    //For attribute initialization
    if(!caller)
    {
        return true;
    }

    auto currentValue = mValues.value(type);
    auto currentTime = QDateTime::currentMSecsSinceEpoch();
    auto lastTimestamp = mRequestTimestamps.value(type, 0);
    if ((lastTimestamp == 0 ||
        (currentTime - lastTimestamp) > 2000) && (!currentValue.isValid() || !areValueUpdatesDisabled()))
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
    mDirectoryIsEmpty = !filesIt.hasNext();
}

void LocalFileFolderAttributes::requestSize(QObject* caller, std::function<void(qint64)> func)
{
    if (requestValue<qint64>(caller, AttributeTypes::SIZE, func))
    {
        initValue<qint64>(AttributeTypes::SIZE, Status::NOT_READY);

        QFileInfo fileInfo(mPath);
        if (fileInfo.exists())
        {
            if (fileInfo.isFile())
            {
                mValues.insert(AttributeTypes::SIZE, fileInfo.size());
            }
            else
            {
                auto future = QtConcurrent::run([this]() -> qint64 {
                    return calculateSize();
                });
                mFolderSizeFuture.setFuture(future);

                // We always send the size, even if the request is async...just to show on GUI a
                // "loading size..." or the most recent size while the new is received
                emit attributeReady(AttributeTypes::SIZE, true);
                return;
            }
        }
        else
        {
            mValues.insert(AttributeTypes::SIZE, static_cast<qint64>(Status::NOT_READABLE));
        }

        emit attributeReady(AttributeTypes::SIZE);
    }
}

void LocalFileFolderAttributes::requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    if (requestValue<QDateTime>(caller, AttributeTypes::MODIFIED_TIME, func))
    {
        QFileInfo fileInfo(mPath);

        if(fileInfo.exists())
        {
            if(fileInfo.isFile())
            {
                mValues.insert(AttributeTypes::MODIFIED_TIME, fileInfo.lastModified());
            }
            //Is local folder
            else
            {
                if (mDirectoryIsEmpty)
                {
                    requestCreatedTime(caller, func);
                }
                else
                {
                    auto future = QtConcurrent::run([this]() -> QDateTime {
                        return calculateModifiedTime();
                    });
                    mModifiedTimeWatcher.setFuture(future);
                    //We always send the time, even if the request is async...just to show on GUI a "loading time..." or the most recent time while the new is received
                    emit attributeReady(AttributeTypes::MODIFIED_TIME, true);
                    return;
                }
            }
        }
        else
        {
            mValues.remove(AttributeTypes::MODIFIED_TIME);
        }
        emit attributeReady(AttributeTypes::MODIFIED_TIME);
    }
}

void LocalFileFolderAttributes::onModifiedTimeCalculated()
{
    auto newModifiedTime(mModifiedTimeWatcher.result());
    mValues.insert(AttributeTypes::MODIFIED_TIME, newModifiedTime);
    emit attributeReady(AttributeTypes::MODIFIED_TIME);
}

void LocalFileFolderAttributes::onSizeCalculated()
{
    mValues.insert(AttributeTypes::SIZE, mFolderSizeFuture.result());
    emit attributeReady(AttributeTypes::SIZE);
}

bool LocalFileFolderAttributes::attributeNeedsUpdate(QObject* caller, int type)
{
    if(mPath.isEmpty())
    {
        return false;
    }

    return FileFolderAttributes::attributeNeedsUpdate(caller, type);
}

void LocalFileFolderAttributes::requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    if (requestValue<QDateTime>(caller, AttributeTypes::CREATED_TIME, func))
    {
        QFileInfo fileInfo(mPath);
        if (fileInfo.exists())
        {
            QVariant& createdTime = mValues[AttributeTypes::CREATED_TIME];
#ifdef Q_OS_WINDOWS
            struct stat result;
            const QString sourcePath = mPath;
            QVarLengthArray<wchar_t, MAX_PATH + 1> file(sourcePath.length() + 2);
            sourcePath.toWCharArray(file.data());
            file[sourcePath.length()] = wchar_t{};
            file[sourcePath.length() + 1] = wchar_t{};
            if (_wstat(file.constData(), &result) == 0)
            {
                createdTime = QVariant::fromValue<QDateTime>(QDateTime::fromSecsSinceEpoch(result.st_ctime));
            }
#elif defined(Q_OS_MACOS)
            struct stat the_time;
            stat(mPath.toUtf8(), &the_time);
            createdTime = QVariant::fromValue<QDateTime>(
                QDateTime::fromSecsSinceEpoch(the_time.st_birthtimespec.tv_sec));
#elif defined(Q_OS_LINUX)
            createdTime = QVariant::fromValue<QDateTime>(QDateTime::fromSecsSinceEpoch(0));
#endif
            if (!fileInfo.isFile())
            {
                if (mDirectoryIsEmpty)
                {
                    mValues.insert(AttributeTypes::MODIFIED_TIME, mValues.value(AttributeTypes::CREATED_TIME));
                }
            }
        }

        emit attributeReady(AttributeTypes::CREATED_TIME);
    }
}

void LocalFileFolderAttributes::requestCRC(QObject *caller, std::function<void (const QString &)> func)
{
    if (requestValue<QString>(caller, AttributeTypes::CRC, func))
    {
        if (!mPath.isEmpty())
        {
            QFileInfo fileInfo(mPath);

            if (fileInfo.exists())
            {
                if (fileInfo.isFile())
                {
                    std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRC(
                        QDir::toNativeSeparators(fileInfo.filePath()).toUtf8().constData()));
                    mValues.insert(AttributeTypes::CRC, QString::fromUtf8(crc.get()));
                }
            }
        }

        emit attributeReady(AttributeTypes::CRC);
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
    if(!fileInfo.isReadable())
    {
        newSize = NOT_READABLE;
    }
    if(!mPath.isEmpty() && fileInfo.exists())
    {
        QDirIterator filesIt(mPath, QDir::Files| QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);

        while (filesIt.hasNext())
        {
            filesIt.next();
            newSize += filesIt.fileInfo().size();
        }
    }

    return newSize;
}

QString LocalFileFolderAttributes::path() const
{
    return mPath;
}

void LocalFileFolderAttributes::setPath(const QString &newPath)
{
    if(mPath != newPath)
    {
        mPath = newPath;
        mValues.clear();
    }
}

//REMOTE
RemoteFileFolderAttributes::RemoteFileFolderAttributes(const QString &filePath, QObject *parent, bool waitForAttributes)
    : FileFolderAttributes(parent),
      mFilePath(filePath),
      mHandle(mega::INVALID_HANDLE),
      mWaitForAttributes(waitForAttributes)

{
}

RemoteFileFolderAttributes::RemoteFileFolderAttributes(mega::MegaHandle handle, QObject* parent, bool waitForAttributes)
    : FileFolderAttributes(parent),
      mHandle(handle),
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
    if (requestValue<qint64>(caller, AttributeTypes::SIZE, func))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if(node)
        {
            if(node->isFile())
            {
                mValues.insert(AttributeTypes::SIZE, std::max(static_cast<long long>(node->getSize()), static_cast<long long>(0)));
            }
            else
            {
                auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
                    this,
                    [this](mega::MegaRequest* request, mega::MegaError* e) {
                        if (request->getType() == mega::MegaRequest::TYPE_FOLDER_INFO
                            && e->getErrorCode() == mega::MegaError::API_OK)
                        {
                            auto folderInfo = request->getMegaFolderInfo();
                            if (folderInfo)
                            {
                                mValues.insert(AttributeTypes::SIZE, std::max(folderInfo->getCurrentSize(), 0LL));
                            }
                        }

                        if (mEventLoop.isRunning())
                        {
                            mEventLoop.quit();
                        }
                        else
                        {
                            emit attributeReady(AttributeTypes::SIZE);
                        }
                    });

                MegaSyncApp->getMegaApi()->getFolderInfo(node.get(), listener.get());

                if (mWaitForAttributes && size() < 0 && !mEventLoop.isRunning())
                {
                    mEventLoop.exec();
                }
                else
                {
                    // We always send the size, even if the request is async...just to show on GUI a
                    // "loading size..." or the most recent size while the new is received
                    emit attributeReady(AttributeTypes::SIZE, true);
                    return;
                }
            }
        }

        emit attributeReady(AttributeTypes::SIZE);
    }
}

void RemoteFileFolderAttributes::requestFileCount(QObject *caller, std::function<void (int)> func)
{
    std::unique_ptr<mega::MegaNode> node = getNode();
    if(node && node->isFolder())
    {
        if (requestValue<int>(caller, RemoteAttributeTypes::FILE_COUNT, func))
        {
            initValue<int>(RemoteAttributeTypes::FILE_COUNT, Status::NOT_READY);

            auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
                this,
                [this](mega::MegaRequest* request, mega::MegaError* e) {
                    if (request->getType() == mega::MegaRequest::TYPE_FOLDER_INFO
                        && e->getErrorCode() == mega::MegaError::API_OK)
                    {
                        auto folderInfo = request->getMegaFolderInfo();
                        if(folderInfo)
                        {
                            mValues.insert(RemoteAttributeTypes::FILE_COUNT, std::max(folderInfo->getNumFiles(), 0));
                        }
                    }

                    if (mEventLoop.isRunning())
                    {
                        mEventLoop.quit();
                    }
                    else
                    {
                        emit attributeReady(RemoteAttributeTypes::FILE_COUNT);
                    }
                });

            MegaSyncApp->getMegaApi()->getFolderInfo(node.get(), listener.get());

            if(mWaitForAttributes && mValues.value(RemoteAttributeTypes::FILE_COUNT).toInt() < 0 && !mEventLoop.isRunning())
            {
                mEventLoop.exec();
            }
            else
            {
                //We always send the file count, even if the request is async...just to show on GUI a "loading file count..." or the most recent file count while the new is received
                emit attributeReady(RemoteAttributeTypes::FILE_COUNT, true);
                return;
            }
            emit attributeReady(RemoteAttributeTypes::FILE_COUNT);
        }
    }
}

void RemoteFileFolderAttributes::requestModifiedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    if (requestValue<QDateTime>(caller, AttributeTypes::MODIFIED_TIME, func))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if(node)
        {
            int64_t newTime = node->isFile() ? node->getModificationTime()
                                     : node->getCreationTime();

            mValues.insert(AttributeTypes::MODIFIED_TIME, QDateTime::fromSecsSinceEpoch(newTime));
        }
        emit attributeReady(AttributeTypes::MODIFIED_TIME);
    }
}

void RemoteFileFolderAttributes::requestCreatedTime(QObject* caller,std::function<void(const QDateTime&)> func)
{
    if (requestValue<QDateTime>(caller, AttributeTypes::CREATED_TIME, func))
    {
        std::unique_ptr<mega::MegaNode> node = getNode(Version::First);
        if(node)
        {
            mValues.insert(AttributeTypes::CREATED_TIME, QDateTime::fromSecsSinceEpoch(node->getCreationTime()));
        }
        emit attributeReady(AttributeTypes::CREATED_TIME);
    }
}

void RemoteFileFolderAttributes::requestCRC(QObject *caller, std::function<void (const QString &)> func)
{
    if (requestValue<QString>(caller, AttributeTypes::CRC, func))
    {
        std::unique_ptr<mega::MegaNode> node = getNode();
        if (node)
        {
            if (const char* fp = node->getFingerprint())
            {
                std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRCFromFingerprint(fp));
                mValues.insert(AttributeTypes::CRC, QString::fromUtf8(crc.get()));
            }
            else
            {
                mValues.remove(AttributeTypes::CRC);
            }
        }

        emit attributeReady(AttributeTypes::CRC);
    }
}

bool RemoteFileFolderAttributes::isCurrentUser() const
{
    return mIsCurrentUser;
}

void RemoteFileFolderAttributes::requestUser(QObject *caller, std::function<void (QString)> func)
{
    std::unique_ptr<mega::MegaNode> node = getNode();
    if (node)
    {
        if (requestValue<QString>(caller, RemoteAttributeTypes::USER, func))
        {
            if (node->getOwner() != MegaSyncApp->getMegaApi()->getMyUserHandleBinary())
            {
                mIsCurrentUser = false;
                mOwner = node->getOwner();

                std::unique_ptr<mega::MegaNode> node = getNode();
                if (node)
                {
                    if (attributeNeedsUpdate(caller, RemoteAttributeTypes::USER))
                    {
                        auto listener =
                            RequestListenerManager::instance().registerAndGetCustomFinishListener(
                                this,
                                [this, caller, func](mega::MegaRequest* request,
                                                     mega::MegaError* e) {
                                    if (e->getErrorCode() == mega::MegaError::API_OK)
                                    {
                                        auto emailFromRequest = request->getEmail();
                                        if (emailFromRequest)
                                        {
                                            mUserEmail = QString::fromUtf8(emailFromRequest);
                                            mUserFullName =
                                                UserAttributes::FullName::requestFullName(
                                                    emailFromRequest);
                                        }
                                    }

                                    if (mUserFullName)
                                    {
                                        if (mUserFullName->isAttributeReady())
                                        {
                                            mValues.insert(RemoteAttributeTypes::USER,
                                                           mUserFullName->getFullName());
                                            emit attributeReady(RemoteAttributeTypes::USER);
                                        }
                                        else
                                        {
                                            this->connect(
                                                mUserFullName.get(),
                                                &UserAttributes::FullName::fullNameReady,
                                                caller,
                                                [this] {
                                                    mValues.insert(RemoteAttributeTypes::USER,
                                                                   mUserFullName->getFullName());
                                                    emit attributeReady(RemoteAttributeTypes::USER);
                                                });
                                        }
                                    }
                                });

                        MegaSyncApp->getMegaApi()->getUserEmail(mOwner, listener.get());

                        // We always send the user, even if the request is async...just to show on
                        // GUI a "loading user..." or the most recent user while the new is received
                        emit attributeReady(RemoteAttributeTypes::USER, true);
                        return;
                    }
                }
            }
            else
            {
                mIsCurrentUser = true;
                mOwner = mega::INVALID_HANDLE;
                mUserEmail.clear();
                mUserFullName = nullptr;
                mValues.remove(RemoteAttributeTypes::USER);
            }
            emit attributeReady(RemoteAttributeTypes::USER);
        }
    }
}

void RemoteFileFolderAttributes::requestVersions(QObject* caller, std::function<void (int)> func)
{
    if (requestValue<int>(caller, RemoteAttributeTypes::VERSIONS, func))
    {
        if (attributeNeedsUpdate(caller, RemoteFileFolderAttributes::VERSIONS))
        {
            std::unique_ptr<mega::MegaNode> node = getNode();
            if (node)
            {
                mValues.insert(RemoteAttributeTypes::VERSIONS,
                               MegaSyncApp->getMegaApi()->getVersions(node.get())->size());
            }
        }

        emit attributeReady(RemoteAttributeTypes::VERSIONS);
    }
}

int RemoteFileFolderAttributes::versionCount()
{
    return mValues.value(RemoteAttributeTypes::VERSIONS, 0).toInt();
}

int RemoteFileFolderAttributes::fileCount()
{
    return mValues.value(RemoteAttributeTypes::FILE_COUNT, 0).toInt();
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

bool RemoteFileFolderAttributes::attributeNeedsUpdate(QObject* caller, int type)
{
    if(mHandle == mega::INVALID_HANDLE)
    {
        return false;
    }

    return FileFolderAttributes::attributeNeedsUpdate(caller, type);

}

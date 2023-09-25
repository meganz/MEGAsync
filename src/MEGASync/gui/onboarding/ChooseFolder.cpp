#include <QDir>
#include <QDebug>

#include "ChooseFolder.h"
#include <Platform.h>
#include <MegaApplication.h>
#include "gui/node_selector/gui/NodeSelectorSpecializations.h"
#include <DialogOpener.h>

QString ChooseLocalFolder::DEFAULT_FOLDER(QString::fromLatin1("MEGA"));
QString ChooseLocalFolder::DEFAULT_FOLDER_PATH(QString::fromLatin1("/") + ChooseLocalFolder::DEFAULT_FOLDER);

ChooseLocalFolder::ChooseLocalFolder(QObject* parent)
    : QObject(parent)
    , mFolderName(DEFAULT_FOLDER)
{
    // The default one (MEGA) should be created if it not exists
    createDefault();
}

void ChooseLocalFolder::openFolderSelector()
{
    SelectorInfo info;
    info.title = tr("Select local folder");
    info.defaultDir = mFolder;
    info.func = [this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            mFolder = (QDir::toNativeSeparators(QDir(fPath).canonicalPath()));
            mFolderName = QDir::fromNativeSeparators(fPath).split(QString::fromLatin1("/")).last().prepend(QString::fromLatin1("/"));
            if(!mFolder.isNull() && !mFolder.isEmpty())
            {
                emit folderChanged();
            }
        }
    };
    Platform::getInstance()->folderSelector(info);
}

const QString ChooseLocalFolder::getFolder()
{
    return mFolder;
}

void ChooseLocalFolder::reset()
{
    mFolderName.clear();
    mFolder.clear();
    emit folderChanged();
}

void ChooseLocalFolder::createDefault()
{
    mFolder = Utilities::getDefaultBasePath();
    mFolder.append(DEFAULT_FOLDER_PATH);
    mFolder = QDir::toNativeSeparators(mFolder);
    QDir defaultFolder(mFolder);
    if (!defaultFolder.exists() && !defaultFolder.mkpath(QString::fromUtf8(".")))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           QString::fromUtf8("ChooseFolder: %1 cannot be created.").arg(mFolder).toUtf8().constData());
    }
}

ChooseRemoteFolder::ChooseRemoteFolder(QObject *parent)
    : QObject(parent)
    , mFolderName(ChooseLocalFolder::DEFAULT_FOLDER_PATH)
    , mFolderHandle(mega::INVALID_HANDLE)
{
}

void ChooseRemoteFolder::openFolderSelector()
{
    QPointer<SyncNodeSelector> nodeSelector = new SyncNodeSelector();

    if(mFolderHandle != mega::INVALID_HANDLE)
    {
        std::shared_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mFolderHandle));
        nodeSelector->setSelectedNodeHandle(node);
    }

    DialogOpener::showDialog<SyncNodeSelector>(nodeSelector, [nodeSelector, this]()
    {
        if (nodeSelector->result() == QDialog::Accepted)
        {
            mFolderHandle = nodeSelector->getSelectedNodeHandle();
            if(auto node = MegaSyncApp->getMegaApi()->getNodeByHandle(mFolderHandle))
            {
                mFolderName = QString::fromUtf8(node->getName());
                mFolderName.prepend(QString::fromLatin1("/"));
                if(!mFolderName.isNull() && !mFolderName.isEmpty())
                {
                    emit folderNameChanged();
                }
            }
        }
    });
}

const mega::MegaHandle ChooseRemoteFolder::getHandle()
{
    return mFolderHandle;
}

void ChooseRemoteFolder::reset()
{
    mFolderHandle = mega::INVALID_HANDLE;
    mFolderName = ChooseLocalFolder::DEFAULT_FOLDER_PATH;
    emit folderNameChanged();
}

const QString ChooseRemoteFolder::getFolderName()
{
    return mFolderName;
}

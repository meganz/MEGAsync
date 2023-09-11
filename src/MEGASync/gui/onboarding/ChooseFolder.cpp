#include <QDir>
#include <QDebug>

#include "ChooseFolder.h"
#include <Platform.h>
#include <MegaApplication.h>
#include "gui/node_selector/gui/NodeSelectorSpecializations.h"
#include <DialogOpener.h>

QString ChooseLocalFolder::DEFAULT_FOLDER(QString::fromUtf8("/MEGA"));

ChooseLocalFolder::ChooseLocalFolder(QObject* parent)
    : QObject(parent)
    , mFolderName(QString())
    , mFolder(QString(DEFAULT_FOLDER))
{
}

void ChooseLocalFolder::openFolderSelector()
{
    Platform::getInstance()->folderSelector(tr("Select local folder"), mFolder, false, nullptr, [this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            mFolder = (QDir::toNativeSeparators(QDir(fPath).canonicalPath()));
            mFolderName = QDir::fromNativeSeparators(fPath).split(QString::fromLatin1("/")).last().prepend(QString::fromLatin1("/"));
            emit folderChanged(mFolder);
        }
    });
}

const QString ChooseLocalFolder::getFolder()
{
    return mFolder;
}

void ChooseLocalFolder::reset()
{
    mFolderName.clear();
    mFolder = DEFAULT_FOLDER;
    emit folderChanged(mFolderName);
}

bool ChooseLocalFolder::createDefault()
{
    bool success = true;
    if(mFolderName.isEmpty())
    {
        QString defaultFolderPath = Utilities::getDefaultBasePath();
        defaultFolderPath.append(DEFAULT_FOLDER);
        defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
        QDir defaultFolder(defaultFolderPath);
        success = defaultFolder.mkpath(QString::fromUtf8("."));
        mFolder = QDir::toNativeSeparators(QDir(defaultFolder).canonicalPath());
    }
    return success;
}

ChooseRemoteFolder::ChooseRemoteFolder(QObject *parent)
    : QObject(parent)
    , mFolderName(ChooseLocalFolder::DEFAULT_FOLDER)
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
                emit folderChanged(mFolderName.prepend(QString::fromLatin1("/")));
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
    mFolderName = QString();
    emit folderChanged(mFolderName);
}

const QString ChooseRemoteFolder::getFolder()
{
    return mFolderName;
}

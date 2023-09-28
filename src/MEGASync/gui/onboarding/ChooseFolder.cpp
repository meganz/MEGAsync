#include <QDir>
#include <QDebug>

#include "ChooseFolder.h"
#include <Platform.h>
#include <MegaApplication.h>
#include "gui/node_selector/gui/NodeSelectorSpecializations.h"
#include <DialogOpener.h>

ChooseLocalFolder::ChooseLocalFolder(QObject* parent)
    : QObject(parent)
{

}

void ChooseLocalFolder::openFolderSelector(const QString& folderPath)
{
    auto openFromFolder = QDir::toNativeSeparators(Utilities::getDefaultBasePath());

    if (!folderPath.isEmpty())
    {
        openFromFolder = QDir::toNativeSeparators(folderPath);
        QDir openFromFolderDir(openFromFolder);
        if (openFromFolderDir.cdUp())
        {
            openFromFolder = openFromFolderDir.path();
        }
        else
        {
            openFromFolder = QDir::toNativeSeparators(Utilities::getDefaultBasePath());
        }
    }

    SelectorInfo info;
    info.title = tr("Select local folder");
    info.defaultDir = openFromFolder;
    info.canCreateDirectoreis = true;
    info.func = [this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            auto folder = QDir::toNativeSeparators(QDir(fPath).canonicalPath());
            if(!folder.isNull() && !folder.isEmpty())
            {
                emit folderChoosen(folder);
            }
        }
    };

    Platform::getInstance()->folderSelector(info);
}

bool ChooseLocalFolder::createFolder(const QString& folderPath)
{
    if (folderPath.isEmpty())
    {
        return false;
    }

    auto folder = QDir::toNativeSeparators(folderPath);

    QDir defaultFolder(folder);
    if (!defaultFolder.exists() && !defaultFolder.mkpath(QString::fromUtf8(".")))
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           QString::fromUtf8("ChooseFolder: %1 cannot be created.").arg(folderPath).toUtf8().constData());

        return false;
    }

    return true;
}

QString ChooseLocalFolder::getDefaultFolder(const QString& folderName)
{
    auto folder = Utilities::getDefaultBasePath();
    if (!folderName.isEmpty())
    {
        folder.append(QString::fromLatin1("/"));
        folder.append(folderName);
    }

    return QDir::toNativeSeparators(folder);
}

QString ChooseRemoteFolder::DEFAULT_FOLDER(QString::fromLatin1("MEGA"));
QString ChooseRemoteFolder::DEFAULT_FOLDER_PATH(QString::fromLatin1("/") + ChooseRemoteFolder::DEFAULT_FOLDER);

ChooseRemoteFolder::ChooseRemoteFolder(QObject *parent)
    : QObject(parent)
    , mFolderHandle(mega::INVALID_HANDLE)
    , mFolderName(ChooseRemoteFolder::DEFAULT_FOLDER_PATH)
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

mega::MegaHandle ChooseRemoteFolder::getHandle()
{
    return mFolderHandle;
}

void ChooseRemoteFolder::reset()
{
    mFolderHandle = mega::INVALID_HANDLE;
    mFolderName = ChooseRemoteFolder::DEFAULT_FOLDER_PATH;
    emit folderNameChanged();
}

QString ChooseRemoteFolder::getFolderName()
{
    return mFolderName;
}

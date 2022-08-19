#include "DuplicatedUploadChecker.h"
#include "DuplicatedNodeDialogs/DuplicatedNodeDialog.h"

#include <Preferences.h>
#include <MegaApplication.h>

#include <QDir>
#include <QFileInfo>

///BASE
///
void DuplicatedUploadBase::onNodeItemSelected()
{
    if(auto nodeItem = dynamic_cast<DuplicatedNodeItem*>(sender()))
    {
        mUploadInfo->setSolution(nodeItem->getType());
        emit selectionDone();
    }
}

std::shared_ptr<DuplicatedNodeInfo> DuplicatedUploadBase::checkUpload(const QString &localPath, std::shared_ptr<mega::MegaNode> parentNode)
{
    QDir dir(localPath);

    auto info = std::make_shared<DuplicatedNodeInfo>();
    info->setLocalPath(localPath);
    info->setParentNode(parentNode);

    auto conflictNode = info->checkNameNode(dir.dirName(), parentNode);
    if(conflictNode)
    {
        info->setRemoteConflictNode(conflictNode);
        info->setHasConflict(true);
    }

    return info;
}

QString DuplicatedUploadBase::getHeader(bool isFile)
{
    return isFile ? DuplicatedNodeDialog::tr("A file named [A] already exists at this destination")
           : DuplicatedNodeDialog::tr("A folder named [A] already exists at this destination");
}

QString DuplicatedUploadBase::getSkipText(bool isFile)
{
    return isFile ? DuplicatedNodeDialog::tr("The file at this destination will be maintained.")
           : DuplicatedNodeDialog::tr("The folder at this destination will be maintained.");
}

////FILE
void DuplicatedUploadFile::fillUi(DuplicatedNodeDialog *dialog, std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    mUploadInfo = conflict;

    dialog->setHeader(getHeader(conflict->isRemoteFile()),conflict->getName());

    auto fileVersioningDisabled(Preferences::instance()->fileVersioningDisabled());

    if(!conflict->haveDifferentType())
    {
        DuplicatedLocalItem* uploadItem = new DuplicatedLocalItem(dialog);

        if(fileVersioningDisabled)
        {
            uploadItem->setInfo(conflict, NodeItemType::FILE_UPLOAD_AND_REPLACE);
            QString description = DuplicatedNodeDialog::tr("The file at this destination will be replaced with the new file.");
            uploadItem->setDescription(description);
        }
        else
        {
            uploadItem->setInfo(conflict, NodeItemType::FILE_UPLOAD_AND_UPDATE);
            uploadItem->setDescription(DuplicatedNodeDialog::tr("The file will be updated with version history."));
            uploadItem->showLearnMore(QLatin1String("https://help.mega.io/files-folders/restore-delete/file-version-history"));
        }

        connect(uploadItem, &DuplicatedNodeItem::actionClicked, this, &DuplicatedUploadFile::onNodeItemSelected);
        dialog->addNodeItem(uploadItem);
    }

    //Upload and merge item
    DuplicatedRenameItem* uploadAndRename = new DuplicatedRenameItem(dialog);
    uploadAndRename->setInfo(conflict);
    uploadAndRename->setDescription(DuplicatedNodeDialog::tr("The file will be renamed as:"));
    connect(uploadAndRename, &DuplicatedNodeItem::actionClicked, this, &DuplicatedUploadFile::onNodeItemSelected);
    dialog->addNodeItem(uploadAndRename);

    DuplicatedRemoteItem* dontUploadItem = new DuplicatedRemoteItem(dialog);
    dontUploadItem->setInfo(conflict, NodeItemType::DONT_UPLOAD);
    dontUploadItem->setDescription(getSkipText(conflict->isRemoteFile()));
    connect(dontUploadItem, &DuplicatedNodeItem::actionClicked, this, &DuplicatedUploadFile::onNodeItemSelected);
    dialog->addNodeItem(dontUploadItem);
}

///FOLDER
void DuplicatedUploadFolder::fillUi(DuplicatedNodeDialog* dialog, std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    mUploadInfo = conflict;

    dialog->setHeader(getHeader(conflict->isRemoteFile()), conflict->getName());

    if(!conflict->haveDifferentType())
    {
        //Upload and merge item
        DuplicatedLocalItem* uploadAndMergeItem = new DuplicatedLocalItem(dialog);
        uploadAndMergeItem->setInfo(conflict, NodeItemType::FOLDER_UPLOAD_AND_MERGE);
        uploadAndMergeItem->setDescription(DuplicatedNodeDialog::tr("The new folder will be merged with the folder at this destination."));
        connect(uploadAndMergeItem, &DuplicatedNodeItem::actionClicked, this, &DuplicatedUploadFolder::onNodeItemSelected);
        dialog->addNodeItem(uploadAndMergeItem);
    }

    DuplicatedRemoteItem* dontUploadItem = new DuplicatedRemoteItem(dialog);
    dontUploadItem->setInfo(conflict, NodeItemType::DONT_UPLOAD);
    dontUploadItem->setDescription(getSkipText(conflict->isRemoteFile()));
    connect(dontUploadItem, &DuplicatedNodeItem::actionClicked, this, &DuplicatedUploadFolder::onNodeItemSelected);
    dialog->addNodeItem(dontUploadItem);
}

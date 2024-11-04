#include "BackupCandidatesFolderSizeRequester.h"

#include "DataController.h"
#include "FileFolderAttributes.h"

BackupCandidatesFolderSizeRequester::BackupCandidatesFolderSizeRequester(QObject* parent):
    QObject(parent)
{}

BackupCandidatesFolderSizeRequester::~BackupCandidatesFolderSizeRequester()
{
    qDeleteAll(mAttributesByFolder);
}

void BackupCandidatesFolderSizeRequester::addFolder(const QString& folder)
{
    // Do not init the LocalFileFolderAttributes with the path, as it checks the QDir
    // And asks for permission even if it is no needed
    auto attributeRequester = new LocalFileFolderAttributes(QString());
    mAttributesByFolder.insert(folder, attributeRequester);

    attributeRequester->setValueUpdatesDisable();
    attributeRequester->setPath(folder);
}

void BackupCandidatesFolderSizeRequester::calculateFolderSize(const QString& folder)
{
    auto attributeRequester(mAttributesByFolder.value(folder));
    requestSize(attributeRequester);
}

void BackupCandidatesFolderSizeRequester::removeFolder(const QString& folder)
{
    auto attributeRequester = mAttributesByFolder.take(folder);
    if (attributeRequester)
    {
        attributeRequester->deleteLater();
    }
}

void BackupCandidatesFolderSizeRequester::requestSize(
    QPointer<LocalFileFolderAttributes> attributeRequester)
{
    if (attributeRequester)
    {
        auto folder(attributeRequester->getPath());
        attributeRequester->requestSize(this,
                                        [this, folder](long long size)
                                        {
                                            emit sizeReceived(folder, size);
                                        });
    }
}

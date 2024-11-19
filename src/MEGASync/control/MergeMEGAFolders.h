#ifndef MERGEMEGAFOLDERS_H
#define MERGEMEGAFOLDERS_H

#include <megaapi.h>
#include <memory>

#include <QObject>

//FOLDER MERGE LOGIC
/*
   1. Detect if there are more than 1 folder with the same name in the name conflict received from the SDK
   2. Get the folder with more files (we can call it the "main" folder) as it is going to be faster moving other folders to this one
   3. Start iterating the rest of folders one by one (we can call them the "secondary" folders:
        a. If the secondary and the main folder have a file with the same name
            i.  If it is identical (same fingerpint), we skip it
            ii. It if is not identical (different fingerprint), I move the secondary folder file with a different name (%1 suffix)
        b. If the secondary folder has a file which is not in the main folder, we move it directly.
        c. If the secondary and the main folder have a folder with the same name:
            i.  We run this algorithm recursively but updating the main and the secondary folders pointer.
        d. If the secondary folder has a folder which is not in the main folder, we move it directly
*/
class MergeMEGAFolders
{
public:
    enum ActionForDuplicates
    {
        Rename,
        IgnoreAndRemove,
        IgnoreAndMoveToBin,
    };

    static std::shared_ptr<mega::MegaError> merge(mega::MegaNode* folderTarget,
                                                  mega::MegaNode* folderToMerge,
                                                  ActionForDuplicates action);

private:
    MergeMEGAFolders() = default;

    static std::shared_ptr<mega::MegaError> performMerge(mega::MegaNode* folderTarget,
                                                         mega::MegaNode* folderToMerge,
                                                         ActionForDuplicates action);
    static std::shared_ptr<mega::MegaError> mergeTwoFolders(ActionForDuplicates action);
    static std::shared_ptr<mega::MegaError> mergeByName(ActionForDuplicates action);

    // Preparation methods
    static void readTargetFolder(
        mega::MegaNode* folderTarget,
        QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithoutNameConflict,
        QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithNameConflict);
    static std::shared_ptr<mega::MegaError> fixTargetFolderNameConflicts(
        const QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithNameConflict,
        ActionForDuplicates action);
    static std::shared_ptr<mega::MegaError> mergeNestedNodesIntoTargetFolder(
        mega::MegaNode* folderTarget,
        mega::MegaNode* folderToMerge,
        QMap<QString, std::shared_ptr<mega::MegaNode>>& targetNodeWithoutNameConflict,
        ActionForDuplicates action);
    static std::shared_ptr<mega::MegaError> finishMerge(mega::MegaNode* folderTarget,
                                                        mega::MegaNode* folderToMerge,
                                                        ActionForDuplicates action);

    // Utilities
    static void logError(std::shared_ptr<mega::MegaError> error);
    static std::shared_ptr<mega::MegaError> rename(mega::MegaNode* nodeToRename,
                                                   mega::MegaNode* parentNode,
                                                   QStringList& itemsBeingRenamed);
    static QString getNodeName(mega::MegaNode* node);
};

#endif // MERGEMEGAFOLDERS_H

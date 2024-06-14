#ifndef MERGEMEGAFOLDERS_H
#define MERGEMEGAFOLDERS_H

#include <megaapi.h>

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
    MergeMEGAFolders(mega::MegaNode* folderTarget, mega::MegaNode* folderToMerge)
        : mFolderTarget(folderTarget),
          mFolderToMerge(folderToMerge),
          mDepth(0)
    {}

    enum ActionForDuplicates
    {
        Rename,
        IgnoreAndRemove,
        IgnoreAndMoveToBin,
    };
    std::shared_ptr<mega::MegaError> merge(ActionForDuplicates action);

private:
    mega::MegaNode* mFolderTarget;
    mega::MegaNode* mFolderToMerge;
    int mDepth;
};

#endif // MERGEMEGAFOLDERS_H

#ifndef CONTEXTMENUDATA_H
#define CONTEXTMENUDATA_H

#include "MEGAinterface.h"

#include <shobjidl_core.h>
#include <string>
#include <vector>
#include <windows.h>

class ContextMenuData
{
public:
    void initialize(IShellItemArray* psiItemArray);
    void reset();

    bool canRequestUpload() const;
    bool canRequestGetLinks() const;
    bool canRemoveFromLeftPane() const;
    bool canViewOnMEGA() const;
    bool canViewVersions() const;
    bool canSync(MegaInterface::SyncType type) const;
    bool isMEGASyncOpen() const;

    void requestUpload();
    void requestGetLinks();
    void removeFromLeftPane();
    void viewOnMEGA();
    void viewVersions();
    void requestSync(MegaInterface::SyncType type);

    int getUnsyncedFolders() const;
    int getUnsyncedFiles() const;
    int getSyncedFolders() const;
    int getSyncedFiles() const;

    bool hasAnyOptionAvailable() const;

protected:
    bool isSynced(int type, int state);
    bool isUnsynced(int state);
    void processPath(const std::wstring& path);

    std::wstring mInLeftPane;
    std::vector<std::wstring> mSelectedPaths;
    std::vector<int> mPathStates;
    std::vector<int> mPathTypes;
    int mSyncedFolders;
    int mSyncedFiles;
    int mSyncedUnknowns;
    int mUnsyncedFolders;
    int mUnsyncedFiles;
    int mUnsyncedUnknowns;
    bool mCanSync;

private:
    bool isThereAnyUnsyncedItem() const;
    bool isThereAnySyncedItem() const;
};

#endif

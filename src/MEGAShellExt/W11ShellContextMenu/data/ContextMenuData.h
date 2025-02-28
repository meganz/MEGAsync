#ifndef CONTEXTMENUDATA_H
#define CONTEXTMENUDATA_H

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
    bool isMEGASyncOpen() const;

    void requestUpload();
    void requestGetLinks();
    void removeFromLeftPane();
    void viewOnMEGA();
    void viewVersions();

    int getUnsyncedFolders() const;
    int getUnsyncedFiles() const;
    int getSyncedFolders() const;
    int getSyncedFiles() const;

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
};

#endif

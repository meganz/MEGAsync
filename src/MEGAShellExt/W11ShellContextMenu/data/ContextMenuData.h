#ifndef CONTEXTMENUDATA_H
#define CONTEXTMENUDATA_H

#include <string>
#include <vector>

class ContextMenuData
{
public:
    ContextMenuData(void);
    virtual ~ContextMenuData(void);

    void initialize(const std::vector<std::wstring>& selectedFiles, bool forceInitialize = false);
    void reset();
    bool isReset() const;

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
    void processFile(const std::wstring& fileOrDirPath);

    bool mIsReset;
    std::wstring mInLeftPane;
    std::vector<std::wstring> mSelectedFiles;
    std::vector<int> mPathStates;
    std::vector<int> mPathTypes;
    int mSyncedFolders, mSyncedFiles, mSyncedUnknowns;
    int mUnsyncedFolders, mUnsyncedFiles, mUnsyncedUnknowns;
};

#endif

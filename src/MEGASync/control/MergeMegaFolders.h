#ifndef MERGEMEGAFOLDERS_H
#define MERGEMEGAFOLDERS_H

#include <megaapi.h>

#include <QObject>

//This class is use to merge two remote folders
class MergeMEGAFolders : public QObject
{
    Q_OBJECT

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

signals:
    void progressIndicator(const QString& nodeName);

private:
    mega::MegaNode* mFolderTarget;
    mega::MegaNode* mFolderToMerge;
    int mDepth;
};

#endif // MERGEMEGAFOLDERS_H

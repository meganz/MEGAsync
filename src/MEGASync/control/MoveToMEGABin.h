#ifndef MOVETOMEGABIN_H
#define MOVETOMEGABIN_H

#include "megaapi.h"

#include <QObject>
#include <QString>

#include <memory>

//This class is used to move a handle to the MEGA bin
class MoveToMEGABin
{
public:
    std::shared_ptr<mega::MegaError> operator()(mega::MegaHandle handle,
                                                const QString& binFolderName,
                                                bool addDateFolder);

    // Moves into an already-resolved bin folder path, as returned by createBinFolder(). Batches
    // use this so every node lands in the folder that was prepared, instead of re-deriving the
    // date folder per node and drifting to a different one when the date changes mid-batch.
    std::shared_ptr<mega::MegaError> operator()(mega::MegaHandle handle,
                                                const QString& binFolderPath);

    // Creates the folder that operator() would move a node into, without moving anything.
    //
    // Callers that are about to bin several nodes at once should do this first. mkDir() looks the
    // path up and then creates what is missing as two separate steps, so concurrent callers each
    // looked before any of them had created and each created its own copy of the folder
    // (SNC-6818). One creation up front leaves them free to run in parallel.
    //
    // Returns the folder path to hand to operator(), or an empty string when the folder could not
    // be prepared. The return is reliable where the SDK error is not: mkDir() can fail while
    // reporting no error, so callers branch on the folder's existence, never on an error object.
    //
    // Blocks on the SDK, so it must not be called on the GUI thread.
    static QString createBinFolder(const QString& binFolderName, bool addDateFolder);

private:
    static QString buildFolderPath(const QString& binFolderName, bool addDateFolder);

    std::shared_ptr<mega::MegaError> moveToBin(mega::MegaHandle handle, const QString& folderPath);
};

#endif // MOVETOMEGABIN_H

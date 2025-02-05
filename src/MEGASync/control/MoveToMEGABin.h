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
    struct MoveToBinError
    {
        std::shared_ptr<mega::MegaError> moveError;
        std::shared_ptr<mega::MegaError> binFolderCreationError;
    };

    static MoveToBinError moveToBin(mega::MegaHandle handle,
                                    const QString& binFolderName,
                                    bool addDateFolder);
};


#endif // MOVETOMEGABIN_H

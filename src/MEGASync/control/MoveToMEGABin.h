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

private:
    std::shared_ptr<mega::MegaError> moveToBin(mega::MegaHandle handle,
                                               const QString& binFolderName,
                                               bool addDateFolder);
};

#endif // MOVETOMEGABIN_H

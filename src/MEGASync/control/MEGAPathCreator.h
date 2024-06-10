#ifndef MEGAPATHCREATOR_H
#define MEGAPATHCREATOR_H

#include <megaapi.h>

#include <memory>
#include <QStringList>
#include <QObject>

//This class is used to create complex paths in MEGA
class MEGAPathCreator : QObject
{
public:
    MEGAPathCreator() = default;
    std::shared_ptr<mega::MegaNode> mkDir(const QString& root, const QString& path, std::shared_ptr<mega::MegaError>& error);

private:
    std::shared_ptr<mega::MegaNode> createFolder(mega::MegaNode *parentNode, const QString& folderName, std::shared_ptr<mega::MegaError>& error);

    QStringList mPathCreated;
};

#endif // MEGAPATHCREATOR_H

#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include "Types.h"

#include <QMap>
#include <QStringList>

namespace DTI
{
    class TokenManager
    {
    public:
        static TokenManager* instance();
        void run();
    private:
        TokenManager();
        FilePathColourMap parseTokenJSON(const QStringList& tokenFilePathsList);
        bool generateTokenFiles(const FilePathColourMap& fileToColourMap);

        QString mCurrentDir;
    };
} // namespace DTI

#endif // TOKENMANAGER_H




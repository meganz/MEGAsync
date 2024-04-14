#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include "Types.h"

#include <QMap>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>

namespace DTI
{
    class TokenManager
    {
    public:
        static TokenManager* instance();
        void run();

    private:
        TokenManager();
        void preserveThemedColourMapsInMemory(const ThemedColourMap& fileToColourMap);
        ThemedColourMap parseColorTokenJSON(const QStringList& colorTokenFilePathsList, const CoreMap& coreMap);
        CoreMap parseCore(const QString& coreFilePath);
        void recurseCore(QString category, const QJsonObject& categoryObject, CoreMap& coreMap);

        QString mCurrentDir;
    };
} // namespace DTI

#endif // TOKENMANAGER_H




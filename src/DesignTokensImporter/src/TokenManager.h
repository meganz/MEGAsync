#ifndef DTI_TOKEN_MANAGER_H
#define DTI_TOKEN_MANAGER_H

#include "Types.h"

#include <QMap>
#include <QFile>
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
        ThemedColorData parseTheme(QFile& designTokensFile, const ColorData& coreData);
        CoreData parseCore(QFile& designTokensFile);
        void recurseCore(QString category, const QJsonObject& coreColors, CoreData& coreData);
        ColorData parseColorTheme(const QJsonObject& jsonThemeObject, const CoreData& colorData);

        QString mCurrentDir;
    };
}

#endif




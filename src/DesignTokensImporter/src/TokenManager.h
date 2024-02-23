#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include "UIClass.h"

#include <QMap>
#include <QStringList>

#include "StyleDefinitions.h"

namespace DTI
{
    class TokenManager
    {
    public:
        enum ObjectNamesID {
            TOKEN,
            GUI_WIN,
            GUI_LINUX,
            GUI_MAC,
            CSS_STYLESHEETS,
            INVALID_ID
        };

        static const QMap<quint32, QString> OBJECT_ID_TO_STRING_MAP;

        static TokenManager* instance();
        void run();
        bool didTokenUIOrCSSFilesChange(const QMap<QString, QMap<QString, QString>>& parsedHashMap);
        QMap<QString, QMap<QString, QString>> readHashFile();
        bool writeHashFile();
        FilePathColourMap parseTokenJSON(const QStringList& tokenFilePathsList);

    private:
        TokenManager();

        bool didFilesChange(const QStringList& filePathList,
                            const QMap<QString, QString>& hashMap);
        bool areHashesMatching(const QStringList& filePathList,
                              const QMap<QString, QString>& hashMap);
        void parseUiFiles(const QStringList& uiFilePathsList,
                          QVector<QSharedPointer<UIClass>>& UIClasses);
        QStringList generateStylesheets(const ColourMap& colourMap,
                                        const QVector<QSharedPointer<UIClass>>& uiClassList,
                                        const QString& saveDirectory);

        QStringList generateWinApplicationStyleJsonFile();


        QStringList mTokenFilePathsList;
        QStringList mWinUIFilePathsList;
        QStringList mLinuxUIFilePathsList;
        QStringList mMacUIFilePathsList;
        QStringList mCSSFiles;
        QVector<QSharedPointer<UIClass>> mWinUIClasses;
        QVector<QSharedPointer<UIClass>> mLinuxUIClasses;
        QVector<QSharedPointer<UIClass>> mMacUIClasses;
    };
} // namespace DTI

#endif // TOKENMANAGER_H




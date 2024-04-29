#ifndef QTWIDGET_STYLE_GENERATOR_H
#define QTWIDGET_STYLE_GENERATOR_H

#include "IThemeGenerator.h"
#include "UIHandler.h"
#include "Types.h"

#include <QObject>
#include <QSharedPointer>

namespace DTI
{
    class QTWIDGETStyleGenerator : public QObject, public IThemeGenerator
    {
        Q_OBJECT

        enum class ObjectNamesID
        {
            TOKEN,
            GUI_WIN,
            GUI_LINUX,
            GUI_MAC,
            CSS_STYLESHEETS,
            INVALID_ID
        };

    public:
        explicit QTWIDGETStyleGenerator(QObject* parent = nullptr);
        void start(const ThemedColorData& fileToColourMap) override;

    private:
        void initialize(const ThemedColorData& fileToColourMap);
        void createDirectories();
        void parseAndStoreUIDesignTokens();
        void parseUiFiles(const QStringList& uiFilePathsList,
                          QVector<QSharedPointer<UIHandler>>& UIClasses);
        bool checkInitialFileConditions();
        QMap<QString, QMap<QString, QString>> readHashFile();
        bool didTokenUIOrCSSFilesChange(const QMap<QString, QMap<QString,
                                        QString>>& parsedHashMap);
        bool didFilesChange(const QStringList& filePathList,
                            const QMap<QString, QString>& hashMap);
        bool areHashesMatching(const QStringList& filePathList,
                               const QMap<QString, QString>& hashMap);
        void generateStyleSheet(const ThemedColorData& fileToColourMap);
        QStringList generateStylesheets(const ColourMap& colourMap,
                                        const QVector<QSharedPointer<UIHandler>>& uiClassList,
                                        const QString& saveDirectory);
        void addStyleSheetToResource();
        bool writeHashFile();

        QString mCurrentDir;
        QString mTokenFilePath;
        QStringList mWinUIFilePathsList;
        QStringList mLinuxUIFilePathsList;
        QStringList mMacUIFilePathsList;
        QStringList mCSSFiles;
        QStringList mWinCSSFiles;
        QStringList mLinuxCSSFiles;
        QStringList mMacCSSFiles;
        QStringList mCSSThemeFolderNameList;
        QVector<QSharedPointer<UIHandler>> mWinDesignTokenUIs;
        QVector<QSharedPointer<UIHandler>> mLinuxDesignTokenUIs;
        QVector<QSharedPointer<UIHandler>> mMacDesignTokenUIs;
        static const QMap<ObjectNamesID, QString> OBJECT_ID_TO_STRING_MAP;
    };
}

#endif // QTWIDGET_STYLE_GENERATOR_H

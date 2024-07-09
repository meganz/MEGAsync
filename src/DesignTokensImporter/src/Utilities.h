#ifndef DESIGN_TOKENS_UTILITIES
#define DESIGN_TOKENS_UTILITIES

#include <QString>
#include <QMap>
#include <QJsonObject>
#include <QVector>

namespace DTI
{
    class Utilities
    {

    public:
         Utilities() = delete;

        enum class Theme
        {
            LIGHT,
            DARK
        };

        static bool createDirectory(const QString& path);
        static QStringList findFilesInDir(const QString& dirPath, const QString& nameFilter = QString::fromLatin1("*.*"), bool findInSubfolders = false);
        static bool isFileValid(const QString& path);
        static void traverseDirectory(const QString& directoryPath, const QStringList& filters, QStringList& filePaths);
        static bool addToResources(const QString& filePath, const QString& qrcPath);
        static QString extractFileName(const QString& filePath);
        static QString extractFileNameNoExtension(const QString& filePath);
        static QString getFileHash(const QString& filePath);
        static bool writeJSONToFile(const QJsonDocument& jsonDoc, const QString& filePath);
        static QString getSubStringBetweenMarkers(const QString& str, const QString& startMarker, const QString& stopMarker);
        static bool areAllStringsPresent(const QStringList& list1, const QStringList& list2);
        static QString themeToString(Utilities::Theme theme);
        static QString resolvePath(const QString& basePath, const QString& relativePath);
        static QString normalizeHexColoursForQtFormat(QString colour);
    };
}


#endif

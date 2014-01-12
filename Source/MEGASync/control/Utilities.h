#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QHash>
#include <QPixmap>
#include <QDir>

//#define SHOW_LOGS
#ifdef SHOW_LOGS
    #define LOG(x) Utils::log(x)
#else
    #define LOG(X)
#endif

class Utilities
{
public:
    static QString getSizeString(unsigned long long bytes);
    static bool verifySyncedFolderLimits(QString path);

private:
    Utilities() {}
    static QHash<QString, QString> extensionIcons;
    static void initializeExtensions();
    static void countFilesAndFolders(QString path, long *numFiles, long *numFolders, long fileLimit, long folderLimit);
    static QPixmap getExtensionPixmap(QString fileName, QString prefix);

//Platform dependent functions
public:

    static QPixmap getExtensionPixmapSmall(QString fileName);
    static QPixmap getExtensionPixmapMedium(QString fileName);
    static QImage createThumbnail(QString imagePath, int size);
    static bool removeRecursively(QDir dir);
    static void log(QString message);
    static void log(const char *message);
};

#endif // UTILITIES_H

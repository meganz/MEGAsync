#ifndef PLATFORMSTRINGS_H
#define PLATFORMSTRINGS_H

#include <QObject>
#include <QString>

class PlatformStrings : QObject
{
    Q_OBJECT
public:
    static QString movedFileToBin();
    static QString fileExplorer();
    static QString exit();
};

#endif // PLATFORMSTRINGS_H

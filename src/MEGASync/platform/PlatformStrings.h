#ifndef PLATFORMSTRINGS_H
#define PLATFORMSTRINGS_H

#include <QObject>
#include <QString>

class PlatformStrings : public QObject
{
public:
    static QString openSettings();
    static QString syncsDisableWarning();
};

#endif // PLATFORMSTRINGS_H

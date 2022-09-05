#ifndef PLATFORMSTRINGS_H
#define PLATFORMSTRINGS_H

#include <QObject>
#include <QString>

class PlatformStrings : QObject
{
    Q_OBJECT
public:
    static QString openSettings();
    static QString syncsDisableWarning();
    static QString cancelSyncsWarning();
};

#endif // PLATFORMSTRINGS_H

#ifndef SYNCTOOLTIPCREATOR_H
#define SYNCTOOLTIPCREATOR_H

#include <QString>
#include <QFontMetrics>

class SyncTooltipCreator
{
public:
    static QString createForLocal(const QString& path);
    static QString createForRemote(const QString& path);

private:
    static QString createFormattedPath(const QString& path, const QString& label);
    static QString createMultilinePath(const QString& path, const QString& label, const QFontMetrics& fm, const QChar& blankChar);

    static const int mMaxWidthInPixels = 480;
};

#endif // SYNCTOOLTIPCREATOR_H

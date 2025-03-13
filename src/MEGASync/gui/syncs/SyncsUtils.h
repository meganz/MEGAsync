#ifndef SYNCS_UTILS_H
#define SYNCS_UTILS_H

#include <QObject>

class SyncsUtils: public QObject
{
    Q_OBJECT

public:
    SyncsUtils() = delete;

    enum SyncStatusCode
    {
        NONE = 0,
        FULL,
        SELECTIVE
    };
    Q_ENUM(SyncStatusCode)
};

#endif // SYNCS_UTILS_H

#ifndef MEGANODENAMES_H
#define MEGANODENAMES_H

#include <QObject>

class MegaNodeNames : public QObject
{
    Q_OBJECT

public:

    MegaNodeNames() = default;
    ~MegaNodeNames() = default;

    static QString getCloudDriveName()
    {
        return tr("Cloud Drive");
    }

    static QString getIncomingSharesName()
    {
        return tr("Incoming shares");
    }

    static QString getBackupsName()
    {
        return tr("Backups");
    }

    static QString getNodeName(const char* name)
    {
        return tr(name);
    }
};

#endif // MEGANODENAMES_H

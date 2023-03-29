#ifndef MEGANODENAMES_H
#define MEGANODENAMES_H

#include <QObject>
#include <QCoreApplication>

#include <megaapi.h>

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

    static QString getRootNodeName(mega::MegaNode* node)
    {
        if(node)
        {
            return tr(node->getName());
        }

        return QString();
    }

    static QString getNodeName(mega::MegaNode* node)
    {
        if(node)
        {
            if(node->isNodeKeyDecrypted())
            {
                return QString::fromUtf8(node->getName());
            }
            else
            {
                return QCoreApplication::translate("MegaError", "Decryption error");
            }
        }

        return QString();
    }
};

#endif // MEGANODENAMES_H

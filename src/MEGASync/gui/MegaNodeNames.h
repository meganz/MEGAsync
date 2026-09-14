#ifndef MEGANODENAMES_H
#define MEGANODENAMES_H

#include "megaapi.h"

#include <QCoreApplication>
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

    static QString getRubbishName()
    {
        return tr("Rubbish bin");
    }

    static QString getUndecryptedFileName()
    {
        return tr("Undecrypted file");
    }

    static QString getUndecryptedFolderName()
    {
        return tr("Undecrypted folder");
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
                if (node->isFile())
                {
                    return getUndecryptedFileName();
                }
                return getUndecryptedFolderName();
            }
        }

        return QString();
    }

    // Node names are chosen by whoever owns the node (a contact, for incoming shares). Use this
    // variant whenever the name is inserted into a rich-text sink (Qt::RichText/AutoText labels,
    // tooltips, HTML templates) so an embedded <img src="\\host\x"> can't trigger a resource load.
    static QString getRichNodeName(mega::MegaNode* node)
    {
        return getNodeName(node).toHtmlEscaped();
    }
};

#endif // MEGANODENAMES_H

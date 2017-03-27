#include <kactionmenu.h>
#include <kfileitem.h>
#include <kfileitemlistproperties.h>
#include <QDir>
#include <QFileInfo>
#include <QString>

#ifndef WITH_KF5
#include <kaction.h>
#include <kdemacros.h>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#else
#include <KPluginFactory>
#include <KPluginLoader>
#include <KIOWidgets/kabstractfileitemactionplugin.h>
#include <QtNetwork/QLocalSocket>
#include <KIOCore/kfileitem.h>
#include <KIOCore/KFileItemListProperties>
#include <QtWidgets/QAction>
#include <QtCore/QDir>
#include <QtCore/QTimer>

#define KAction QAction
#define KIcon QIcon

#endif

#include "megasync-plugin.h"

K_PLUGIN_FACTORY(MEGASyncPluginFactory, registerPlugin<MEGASyncPlugin>();)
K_EXPORT_PLUGIN(MEGASyncPluginFactory("megasync-plugin"))


typedef enum {
    STRING_UPLOAD = 0,
    STRING_GETLINK = 1,
    STRING_SHARE = 2,
    STRING_SEND = 3
} StringID;

enum {
    FILE_ERROR = 0,
    FILE_SYNCED = 1,
    FILE_PENDING = 2,
    FILE_SYNCING = 3,
    FILE_NOTFOUND = 9,
};

const char OP_PATH_STATE  = 'P'; //Path state
const char OP_INIT        = 'I'; //Init operation
const char OP_END         = 'E'; //End operation
const char OP_UPLOAD      = 'F'; //File-Folder upload
const char OP_LINK        = 'L'; //paste Link
const char OP_SHARE       = 'S'; //Share folder
const char OP_SEND        = 'C'; //Copy to user
const char OP_STRING      = 'T'; //Get Translated String

MEGASyncPlugin::MEGASyncPlugin(QObject* parent, const QList<QVariant> & args):
    KAbstractFileItemActionPlugin(parent)
{
    Q_UNUSED(args);
    sockPath = QDir::home().path();
    sockPath.append(QDir::separator()).append(".local/share/data/Mega Limited/MEGAsync/mega.socket");
    sock.connectToServer(sockPath);
}

MEGASyncPlugin::~MEGASyncPlugin()
{
    sock.close();
}

QList<QAction*> MEGASyncPlugin::actions(const KFileItemListProperties & fileItemInfos, QWidget * parentWidget)
{
    Q_UNUSED(parentWidget);
    QList<QAction*> actions;
    int state;

    int syncedFiles, syncedFolders, unsyncedFiles, unsyncedFolders;
    syncedFiles = syncedFolders = unsyncedFiles = unsyncedFolders = 0;

    for( int i = 0; i < fileItemInfos.items().count(); i++)
    {
        KFileItem item = fileItemInfos.items().at(i);
        selectedFilePath = item.localPath();
        selectedFilePaths << selectedFilePath;

        // get the state of selected file
        state = getState();

        // count the number of synced / unsynced files and folders
        if (state == FILE_SYNCED || state == FILE_SYNCING || state == FILE_PENDING)
        {
            if (item.isDir())
            {
                syncedFolders++;
            }
            else
            {
                syncedFiles++;
            }
        }
        else
        {
            if (item.isDir())
            {
                unsyncedFolders++;
            }
            else
            {
                unsyncedFiles++;
            }
        }
    }

    // populate Menu
    KActionMenu * menuAction = new KActionMenu(this);
    menuAction->setText("MEGA");
    actions << menuAction;

    // if there any unsynced files / folders selected
    if (unsyncedFiles || unsyncedFolders)
    {
        QAction *act = new KAction(this);
        QString actionText = getString(STRING_UPLOAD, unsyncedFiles, unsyncedFolders);
        act->setText(actionText);

        menuAction->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(uploadFiles()));
    }

    // if there any synced files / folders selected
    if (syncedFiles || syncedFolders)
    {
        QAction *act = new KAction(this);
        QString actionText = getString(STRING_GETLINK, syncedFiles, syncedFolders);
        act->setText(actionText);

        menuAction->addAction(act);

        // set menu icon //TODO: state refers to the last file. Does it make any sense??
        if (state == FILE_SYNCED)
            act->setIcon(KIcon("mega-synced"));
        else if (state == FILE_PENDING)
            act->setIcon(KIcon("mega-pending"));
        else if (state == FILE_SYNCING)
            act->setIcon(KIcon("mega-syncing"));

        connect(act, SIGNAL(triggered()), this, SLOT(getLinks()));
    }

    return actions;
}

int MEGASyncPlugin::getState()
{
    QString res;
    res = sendRequest(OP_PATH_STATE, selectedFilePath);
    return res.toInt();
}

void MEGASyncPlugin::getLink()
{
    if (sendRequest(OP_LINK, selectedFilePath).size())
    {
        sendRequest(OP_END, " ");
    }
}

void MEGASyncPlugin::getLinks()
{
    for(QString &path : selectedFilePaths)
    {
        if (sendRequest(OP_LINK, path).size())
        {
        }
    }
    sendRequest(OP_END, " ");
}

void MEGASyncPlugin::uploadFile()
{
    if (sendRequest(OP_UPLOAD, selectedFilePath).size())
    {
        sendRequest(OP_END, " ");
    }
}

void MEGASyncPlugin::uploadFiles()
{
    for(QString &path : selectedFilePaths)
    {
        if (sendRequest(OP_UPLOAD, path).size())
        {
        }
    }
    sendRequest(OP_END, " ");
}

QString MEGASyncPlugin::getString(int type, int numFiles,int numFolders)
{
    QString res;
    QString queryString = "";
    queryString.sprintf("%d:%d:%d", type, numFiles, numFolders);

    res = sendRequest(OP_STRING, queryString);
    return res;
}


// send request and receive response from Extension server
// Return newly-allocated response string
QString MEGASyncPlugin::sendRequest(char type, QString command)
{
    int waitTime = -1; //this makes dolphin hang until the location for an upload is selected. Otherwise megayns segafaults accesing slient socket
    QString req;
    QString out;

    if(!sock.isOpen()) {
        sock.connectToServer(sockPath);
        if(!sock.waitForConnected(waitTime))
            return QString();
    }

    req.sprintf("%c:%s", type, command.toStdString().c_str());

    sock.write(req.toUtf8());
    sock.flush();

    if(!sock.waitForReadyRead(waitTime)) {
        sock.close();
        return QString();
    }

    QString reply;
    reply.append(sock.readAll());

    return reply;
}

#include "megasync-plugin.moc"

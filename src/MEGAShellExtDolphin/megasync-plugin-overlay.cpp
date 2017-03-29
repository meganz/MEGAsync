
#ifndef WITH_KF5
//TODO: actually it makes no sense to have overlayplugin without KF5, since it seems to be available only after >= 5.16
#include <koverlayiconplugin.h>
#include <KPluginFactory>
#include <KIOCore/kfileitem.h>
#else
#include <KF5/KIOWidgets/KOverlayIconPlugin>
#include <KPluginFactory>
#include <KF5/KIOCore/kfileitem.h>
//#include <QLocalServer>
#endif

#include <QtNetwork/QLocalSocket>
#include <QDir>

typedef enum {
    FILE_ERROR = 0,
    FILE_SYNCED = 1,
    FILE_PENDING = 2,
    FILE_SYNCING = 3,
    FILE_NOTFOUND = 9,
} FileState;

const char OP_PATH_STATE  = 'P'; //Path state
const char OP_INIT        = 'I'; //Init operation
const char OP_END         = 'E'; //End operation
const char OP_UPLOAD      = 'F'; //File-Folder upload
const char OP_LINK        = 'L'; //paste Link
const char OP_SHARE       = 'S'; //Share folder
const char OP_SEND        = 'C'; //Copy to user
const char OP_STRING      = 'T'; //Get Translated String

class MegasyncDolphinOverlayPlugin : public KOverlayIconPlugin
{
    Q_PLUGIN_METADATA(IID "com.megasync.ovarlayiconplugin" FILE "megasync-plugin-overlay.json")
    Q_OBJECT

    typedef QHash<QByteArray, QByteArray> StatusMap;
    StatusMap m_status;
    QLocalSocket sockNotifyServer;
    QString sockPathNofityServer;

    QLocalSocket sockExtServer;
    QString sockPathExtServer;

    //TODO: delete the following
    void printinFile(QString what)
    {
        QFile f("/tmp/dbgoverlay.txt");
        if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
            f.write("-->");
            f.write(what.toStdString().c_str());
            f.write("\n");
            f.close();
        }
    }

private slots:
    void notifiedfromServer()
    {
        qDebug("at notifiedfromServer");
        printinFile("at notifiedfromServer");

        while(sockNotifyServer.canReadLine())
        {

            char type[1];
            sockNotifyServer.read(type, 1); //TODO: control errors

            printinFile(type);

            switch(*type) {
            case 'P': // item state changed
                printinFile("item state changed: ");
                //                mega_ext_on_item_changed(mega_ext, p);
                break;
            case 'A': // sync folder added
                //                mega_ext_on_sync_add(mega_ext, p);
                //                mega_ext->syncs_received = TRUE;
                break;
            case 'D': // sync folder deleted
                //                mega_ext_on_sync_del(mega_ext, p);
                break;
            default:
                //                g_warning("Failed to read data!");
                //                g_free(in_line);
                //                mega_notify_client_destroy(mega_ext);
                //                // start connection timer
                //                mega_notify_client_timer_start(mega_ext);
                //                return FALSE;
                break;
            }
            QString url = sockNotifyServer.readLine();
            while(url.endsWith('\n')) url.chop(1);

            printinFile("at notifiedfromServer. url:");
            printinFile(url);
            qDebug("%s",url.toStdString().c_str());

            emit overlaysChanged(QUrl::fromLocalFile(url), getOverlays(QUrl::fromLocalFile(url)));
        }
    }

public:

    MegasyncDolphinOverlayPlugin()
    {
        printinFile("at MegasyncDolphinOverlayPlugin 0001");

        sockPathNofityServer = QDir::home().path();
        sockPathNofityServer.append(QDir::separator()).append(".local/share/data/Mega Limited/MEGAsync/notify.socket");
        sockNotifyServer.connectToServer(sockPathNofityServer);
        printinFile("at MegasyncDolphinOverlayPlugin 0010");

        sockPathExtServer = QDir::home().path();
        sockPathExtServer.append(QDir::separator()).append(".local/share/data/Mega Limited/MEGAsync/mega.socket");
        sockExtServer.connectToServer(sockPathExtServer);

//        connect(m_socket, SIGNAL(connected()), this, SLOT(socket_connected()));
//        connect(m_socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));

        connect(&sockNotifyServer, SIGNAL(readyRead()), this, SLOT(notifiedfromServer()));
        connect(&sockNotifyServer, SIGNAL(error(QLocalSocket::LocalSocketError)),
                this, SLOT(socket_error(QLocalSocket::LocalSocketError)));

        printinFile("at MegasyncDolphinOverlayPlugin 0020");
    }



    ~MegasyncDolphinOverlayPlugin()
    {
        sockNotifyServer.close();
    }

    QStringList getOverlays(const QUrl& url) override
    {
        if (!url.isLocalFile())
            return QStringList();
        QDir localPath(url.toLocalFile());
        const QByteArray localFile = localPath.canonicalPath().toUtf8();

//        helper->sendCommand(QByteArray("RETRIEVE_FILE_STATUS:" + localFile + "\n"));

        printinFile(QStringLiteral("at getOverlays 0020: %1").arg(url.toLocalFile()));

        QStringList r;

        int state = getState(url.toLocalFile());

        printinFile(QStringLiteral("at getOverlays 0030: %1 state: %2").arg(url.toLocalFile()).arg(state));

        switch (state)
        {
            case FILE_SYNCED:
                r << "mega-synced";
                break;
            case FILE_PENDING:
                r << "mega-pending";
                break;
            case FILE_SYNCING:
                r << "mega-syncing";
                break;
            default:
                break;
        }

        return r;

        printinFile(url.toString());
        return QStringList();
    }

private:

    int getState(QString path)
    {
        QString res;
        res = sendRequest(OP_PATH_STATE, path);
        return res.toInt();
    }

    // send request and receive response from Extension server
    // Return newly-allocated response string
    QString sendRequest(char type, QString command)
    {
        int waitTime = -1; //this makes dolphin hang until the location for an upload is selected. Otherwise megayns segafaults accesing slient socket
        QString req;

        if(!sockExtServer.isOpen()) {
            sockExtServer.connectToServer(sockPathExtServer);
            if(!sockExtServer.waitForConnected(waitTime))
                return QString();
        }

        req.sprintf("%c:%s", type, command.toStdString().c_str());

        sockExtServer.write(req.toUtf8());
        sockExtServer.flush();

        if(!sockExtServer.waitForReadyRead(waitTime)) {
            sockExtServer.close();
            return QString();
        }

        QString reply;
        reply.append(sockExtServer.readAll());

        return reply;
    }

};

#include "megasync-plugin-overlay.moc"

#include <KFileItem>
#include <KOverlayIconPlugin>
#include <KPluginFactory>

#include <QDir>
#include <QMetaEnum>
#include <QStandardPaths>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QLocalSocket>

typedef enum {
    RESPONSE_SYNCED  = 0,
    RESPONSE_PENDING = 1,
    RESPONSE_SYNCING = 2,
    RESPONSE_IGNORED = 3,
    RESPONSE_PAUSED  = 4,
    RESPONSE_DEFAULT = 9,
    RESPONSE_ERROR   = 10,
} FileState;

const char OP_PATH_STATE  = 'P'; //Path state
const char OP_INIT        = 'I'; //Init operation
const char OP_END         = 'E'; //End operation
const char OP_UPLOAD      = 'F'; //File-Folder upload
const char OP_LINK        = 'L'; //paste Link
const char OP_SHARE       = 'S'; //Share folder
const char OP_SEND        = 'C'; //Copy to user
const char OP_STRING      = 'T'; //Get Translated String
const char OP_VIEW        = 'V'; //View on MEGA
const char OP_PREVIOUS    = 'R'; //View previous versions

class MegasyncDolphinOverlayPlugin : public KOverlayIconPlugin
{
    Q_PLUGIN_METADATA(IID "io.mega.megasync-plugin-overlay" FILE "megasync-plugin-overlay.json")

    Q_OBJECT

    typedef QHash<QByteArray, QByteArray> StatusMap;
    StatusMap m_status;
    QLocalSocket sockNotifyServer;
    QString sockPathNofityServer;

    QLocalSocket sockExtServer;
    QString sockPathExtServer;

private Q_SLOTS:

    void sockNotifyServer_connected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: connected to Notify Server");
    }

    void sockNotifyServer_disconnected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: disconnected from Notify Server");
    }

    void sockNotifyServer_error(QLocalSocket::LocalSocketError err)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
        qCritical("MEGASYNCOVERLAYPLUGIN: error in connection to notify server: %s", metaEnum.valueToKey(err));
    }

    void sockExtServer_connected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: connected to Ext Server");
    }

    void sockExtServer_disconnected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: disconnected from Ext Server");
    }

    void sockExtServer_error(QLocalSocket::LocalSocketError err)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
        qCritical("MEGASYNCOVERLAYPLUGIN: error in connection to ext server: %s", metaEnum.valueToKey(err));
    }

    void notifiedfromServer()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: notifiedfromServer");

        while(sockNotifyServer.canReadLine())
        {

            char type[1];
            sockNotifyServer.read(type, 1); //TODO: control errors

            QString action = QLatin1String("unknown");

            switch(*type) {
            case 'P': // item state changed
                action = QLatin1String("item state changed");
                break;
            case 'A': // sync folder added
                action = QLatin1String("sync folder added");
                break;
            case 'D': // sync folder deleted
                action = QLatin1String("sync folder deleted");
                break;
            default:
                qCritical("MEGASYNCOVERLAYPLUGIN: unexpected read from notifyServer. type=%s", type);
                break;
            }

            QString url = QString::fromLatin1(sockNotifyServer.readLine());
            while (url.endsWith(QLatin1Char('\n')))
                url.chop(1);

            qDebug("MEGASYNCOVERLAYPLUGIN: Server notified <%s>: %s",action.toUtf8().constData(), url.toUtf8().constData());

            Q_EMIT overlaysChanged(QUrl::fromLocalFile(url), getOverlays(QUrl::fromLocalFile(url)));
        }
    }

public:

    MegasyncDolphinOverlayPlugin()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: Loading plugin ... ");

        connect(&sockNotifyServer, SIGNAL(connected()), this, SLOT(sockNotifyServer_connected()));
        connect(&sockNotifyServer, SIGNAL(disconnected()), this, SLOT(sockNotifyServer_disconnected()));

        connect(&sockNotifyServer, SIGNAL(readyRead()), this, SLOT(notifiedfromServer()));
        connect(&sockNotifyServer, SIGNAL(error(QLocalSocket::LocalSocketError)),
                this, SLOT(sockNotifyServer_error(QLocalSocket::LocalSocketError)));

        connect(&sockExtServer, SIGNAL(connected()), this, SLOT(sockExtServer_connected()));
        connect(&sockExtServer, SIGNAL(disconnected()), this, SLOT(sockExtServer_disconnected()));
        connect(&sockExtServer, SIGNAL(error(QLocalSocket::LocalSocketError)),
                this, SLOT(sockExtServer_error(QLocalSocket::LocalSocketError)));

        sockPathNofityServer = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        sockPathNofityServer.append(QDir::separator())
            .append(QLatin1String("data/Mega Limited/MEGAsync/notify.socket"));
        sockNotifyServer.connectToServer(sockPathNofityServer);

        sockPathExtServer = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        sockPathExtServer.append(QDir::separator())
            .append(QLatin1String("data/Mega Limited/MEGAsync/mega.socket"));
        sockExtServer.connectToServer(sockPathExtServer);
    }

    ~MegasyncDolphinOverlayPlugin()
    {
        sockNotifyServer.close();
    }

    QStringList getOverlays(const QUrl& url) override
    {
        if (!url.isLocalFile())
        {
            return QStringList();
        }

        QStringList r;

        int state = getState(url.toLocalFile());

        switch (state)
        {
            case RESPONSE_SYNCED:
                r << QLatin1String("mega-dolphin-synced");
                qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-synced",url.toLocalFile().toUtf8().constData());
                break;
            case RESPONSE_PENDING:
                r << QLatin1String("mega-dolphin-pending");
                qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-pending",url.toLocalFile().toUtf8().constData());
                break;
            case RESPONSE_SYNCING:
                r << QLatin1String("mega-dolphin-syncing");
                qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-syncing",url.toLocalFile().toUtf8().constData());
                break;
            default:
                qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: %d",url.toLocalFile().toUtf8().constData(),state);
                break;
        }

        return r;
    }

private:

    int getState(QString path)
    {
        QString res;
        res = sendRequest(OP_PATH_STATE, QFileInfo(path).canonicalFilePath());

        return res.isEmpty() ? RESPONSE_ERROR : res.toInt();
    }

    // send request and receive response from Extension server
    // Return newly-allocated response string
    QString sendRequest(char type, QString command)
    {
        int waitTime = -1; // This (instead of a timeout) makes dolphin hang until the location for
                           // an upload is selected  (will be corrected in megasync>3.0.1).
                           // Otherwise megaync segafaults accesing client socket

        if(!sockExtServer.isOpen()) {
            sockExtServer.connectToServer(sockPathExtServer);
            if(!sockExtServer.waitForConnected(waitTime))
                return QString();
        }

        QString req = QString::fromLatin1("%1:%2").arg(type).arg(command);

        sockExtServer.write(req.toUtf8());
        sockExtServer.flush();

        if(!sockExtServer.waitForReadyRead(waitTime)) {
            sockExtServer.close();
            return QString();
        }

        QString reply;
        reply.append(QLatin1String(sockExtServer.readAll()));

        return reply;
    }
};

#include "megasync-plugin-overlay.moc"

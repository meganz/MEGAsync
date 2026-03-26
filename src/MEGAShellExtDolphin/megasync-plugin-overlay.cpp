#include <KFileItem>
#include <KOverlayIconPlugin>
#include <KPluginFactory>

#include <KConfigGroup>
#include <KDesktopFile>
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

    static void sockNotifyServer_connected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: connected to Notify Server");
    }

    static void sockNotifyServer_disconnected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: disconnected from Notify Server");
    }

    static void sockNotifyServer_error(QLocalSocket::LocalSocketError err)
    {
        const auto metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
        qCritical("MEGASYNCOVERLAYPLUGIN: error in connection to notify server: %s", metaEnum.valueToKey(err));
    }

    static void sockExtServer_connected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: connected to Ext Server");
    }

    static void sockExtServer_disconnected()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: disconnected from Ext Server");
    }

    static void sockExtServer_error(QLocalSocket::LocalSocketError err)
    {
        const auto metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
        qCritical("MEGASYNCOVERLAYPLUGIN: error in connection to ext server: %s", metaEnum.valueToKey(err));
    }

    void notifiedfromServer()
    {
        // qDebug("MEGASYNCOVERLAYPLUGIN: notifiedfromServer");

        while(sockNotifyServer.canReadLine())
        {
            char type[1];
            sockNotifyServer.read(type, 1); //TODO: control errors

            // QString action = QLatin1String("unknown");

            // switch(*type) {
            // case 'P': // item state changed
            //     action = QLatin1String("item state changed");
            //     break;
            // case 'A': // sync folder added
            //     action = QLatin1String("sync folder added");
            //     break;
            // case 'D': // sync folder deleted
            //     action = QLatin1String("sync folder deleted");
            //     break;
            // default:
            //     qCritical("MEGASYNCOVERLAYPLUGIN: unexpected read from notifyServer. type=%s", type);
            //     break;
            // }

            const auto url = QString::fromUtf8(sockNotifyServer.readLine().trimmed());

            // qDebug("MEGASYNCOVERLAYPLUGIN: Server notified <%s>: %s",action.toUtf8().constData(), url.toUtf8().constData());

            Q_EMIT overlaysChanged(QUrl::fromLocalFile(url), getOverlays(QUrl::fromLocalFile(url)));
        }
    }

public:

    MegasyncDolphinOverlayPlugin()
    {
        qDebug("MEGASYNCOVERLAYPLUGIN: Loading plugin ... ");

        connect(&sockNotifyServer, &QLocalSocket::connected, this, &MegasyncDolphinOverlayPlugin::sockNotifyServer_connected);
        connect(&sockNotifyServer, &QLocalSocket::disconnected, this, &MegasyncDolphinOverlayPlugin::sockNotifyServer_disconnected);

        connect(&sockNotifyServer,  &QLocalSocket::readyRead, this, &MegasyncDolphinOverlayPlugin::notifiedfromServer);
        connect(&sockNotifyServer,  &QLocalSocket::errorOccurred, this, &MegasyncDolphinOverlayPlugin::sockNotifyServer_error);

        connect(&sockExtServer, &QLocalSocket::connected, this, &MegasyncDolphinOverlayPlugin::sockExtServer_connected);
        connect(&sockExtServer, &QLocalSocket::disconnected, this, &MegasyncDolphinOverlayPlugin::sockExtServer_disconnected);
        connect(&sockExtServer, &QLocalSocket::errorOccurred, this, &MegasyncDolphinOverlayPlugin::sockExtServer_error);

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
            return {};
        }

        auto restoreDefaultIconIfNeeded = [&](const QString& qStrURL/*, int state*/)
        {
            // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: %d",
            //        qStrURL.toUtf8().constData(),
            //        state);
            const auto icon_path = getFolderIconPath(qStrURL);
            if (!icon_path.isEmpty())
            {
                // qDebug() << "MEGASYNCOVERLAYPLUGIN: icon_path =" << icon_path;
                if (icon_path == QLatin1String("mega") ||
                    (icon_path.contains(QLatin1String("/usr/share/icons")) &&
                     icon_path.contains(QLatin1String("apps/mega.png"))))
                {
                    // qDebug() << "MEGASYNCOVERLAYPLUGIN: Restore " << qStrURL << " to default icon";
                    // Restore to default icon
                    changeFolderIcon(qStrURL, QString());
                }
            }
        };

        QStringList r;

        const auto qStrURL = url.toLocalFile();
        const auto state = getState(qStrURL);

        switch (state)
        {
            case RESPONSE_SYNCED:
                r << QLatin1String("mega-dolphin-synced");
                // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-synced", cStrURL);
                break;
            case RESPONSE_PENDING:
                r << QLatin1String("mega-dolphin-pending");
                // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-pending", cStrURL);
                break;
            case RESPONSE_SYNCING:
                r << QLatin1String("mega-dolphin-syncing");
                // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-syncing", cStrURL);
                break;
            case RESPONSE_DEFAULT:
                // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-default", cStrURL);
                // Fallthrough
            case RESPONSE_IGNORED:
                // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-ignored", cStrURL);
                restoreDefaultIconIfNeeded(qStrURL/*, state*/);
                break;

            case RESPONSE_ERROR:
            default:
            {
                // Don't change or remove the icon if MEGASync is not running
                // qDebug("MEGASYNCOVERLAYPLUGIN: getOverlays <%s>: mega-dolphin-error", cStrURL);
                break;
            }
        }

        return r;
    }

private:

    int getState(const QString& path)
    {
        QString res;
        res = sendRequest(OP_PATH_STATE, QFileInfo(path).canonicalFilePath());

        return res.isEmpty() ? RESPONSE_ERROR : res.toInt();
    }

    // send request and receive response from Extension server
    // Return newly-allocated response string
    QString sendRequest(char type, const QString& command)
    {
        const int waitTime = -1; // This (instead of a timeout) makes dolphin hang until the location for
                           // an upload is selected  (will be corrected in megasync>3.0.1).
                           // Otherwise megaync segafaults accesing client socket

        if(!sockExtServer.isOpen())
        {
            sockExtServer.connectToServer(sockPathExtServer);
            if(!sockExtServer.waitForConnected(waitTime))
            {
                return {};
            }
        }

        const auto req = QString::fromLatin1("%1:%2").arg(type).arg(command);

        sockExtServer.write(req.toUtf8());
        sockExtServer.flush();

        if(!sockExtServer.waitForReadyRead(waitTime))
        {
            sockExtServer.close();
            return {};
        }

        // Answer is latin1 encoded
        return QString::fromLatin1(sockExtServer.readAll());
    }

    // Function to get folder icon path
    static QString getFolderIconPath(const QString& folderPath)
    {
        if (folderPath.isEmpty())
        {
            return {};
        }

        QString iconPath;
        KDesktopFile desktopFile(folderPath + QLatin1String("/.directory"));
        if (desktopFile.hasGroup(QLatin1String("Desktop Entry")))
        {
            KConfigGroup group = desktopFile.group(QLatin1String("Desktop Entry"));
            iconPath = group.readEntry(QLatin1String("Icon"), QString());
        }

        return iconPath;
    }

    static void changeFolderIcon(const QString& folderPath, const QString& iconPath)
    {
        if (folderPath.isEmpty())
        {
            // qDebug() << "MegasyncDolphinOverlayPlugin::changeFolderIcon - Folder path is empty. "
            //             "Aborting icon change.";
            return;
        }

        QString desktopFilePath = folderPath + QLatin1String("/.directory");
        KDesktopFile desktopFile(desktopFilePath);

        if (desktopFile.hasGroup(QLatin1String("Desktop Entry")))
        {
            KConfigGroup group = desktopFile.group(QLatin1String("Desktop Entry"));

            if (iconPath.isEmpty())
            {
                group.deleteEntry(QLatin1String("Icon"));
                // qDebug() << "MegasyncDolphinOverlayPlugin::changeFolderIcon - Icon removed.";

                // Ensure changes are written to the file
                group.sync();

                QFile desktopFileHandle(desktopFilePath);
                // Check if the file is empty after syncing changes
                const bool fileIsEmpty = desktopFileHandle.size() == 0;

                if (fileIsEmpty)
                {
                    // qDebug() << "MegasyncDolphinOverlayPlugin::changeFolderIcon - .directory file "
                    //             "is empty, removing it.";
                    desktopFileHandle.remove();
                }
                else
                {
                    // qDebug() << "MegasyncDolphinOverlayPlugin::changeFolderIcon - .directory file "
                    //             "is not empty, leaving it.";
                }
            }
            else
            {
                group.writeEntry(QLatin1String("Icon"), iconPath);
                // qDebug() << "MegasyncDolphinOverlayPlugin::changeFolderIcon - Icon set to:"
                //          << iconPath;
                // Ensure changes are written to the file
                group.sync();
            }
        }
    }
};

#include "megasync-plugin-overlay.moc"

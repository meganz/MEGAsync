#include <kaction.h>
#include <kactionmenu.h>
#include <kdemacros.h>
#include <kfileitem.h>
#include <kfileitemlistproperties.h>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>

#include "megasync-plugin.h"

K_PLUGIN_FACTORY(MEGASyncPluginFactory, registerPlugin<MEGASyncPlugin>();)
K_EXPORT_PLUGIN(MEGASyncPluginFactory("megasync-plugin"))

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

MEGASyncPlugin::MEGASyncPlugin(QObject* parent, const QVariantList & args):
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

    // multiple selection is not supported
    if (fileItemInfos.items().count() != 1) {
        return actions;
    }

    // get the first item
    KFileItem item = fileItemInfos.items().first();
    selectedFilePath = item.localPath();
    QFileInfo itemFileInfo = QFileInfo(selectedFilePath);

    // skip non local file
    if (!item.isLocalFile()) {
        return actions;
    }

    // get the state of selected file
    state = getState();
    if (state == FILE_ERROR) {
        return actions;
    }

    // populate Menu
    KActionMenu * menuAction = new KActionMenu(this);
    menuAction->setText("MEGA");
    actions << menuAction;

    if (state == FILE_SYNCED || state == FILE_SYNCING || state == FILE_PENDING) {
        QAction *act = new KAction(this);
        act->setText("Get MEGA link");
        menuAction->addAction(act);

        // set menu icon
        if (state == FILE_SYNCED)
            act->setIcon(KIcon("mega-synced"));
        else if (state == FILE_PENDING)
            act->setIcon(KIcon("mega-pending"));
        else if (state == FILE_SYNCING)
            act->setIcon(KIcon("mega-syncing"));

        connect(act, SIGNAL(triggered()), this, SLOT(getLink()));
    } else {
        QAction *act = new KAction(this);
        act->setText("Upload files to you MEGA account");
        menuAction->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(uploadFile()));
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
    sendRequest(OP_LINK, selectedFilePath);
}

void MEGASyncPlugin::uploadFile()
{
    if (sendRequest(OP_UPLOAD, selectedFilePath).size())
    {
        sendRequest(OP_END, " ");
    }
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

#ifndef _MEGA_SYNC_PLUGIN_H_
#define _MEGA_SYNC_PLUGIN_H_

#ifndef WITH_KF5
#include <kabstractfileitemactionplugin.h>
#else
#include <KIOWidgets/kabstractfileitemactionplugin.h>
#endif
#include <kactionmenu.h>
#include <QLocalSocket>

class MEGASyncPlugin: public KAbstractFileItemActionPlugin
{
    Q_OBJECT
private:
    QLocalSocket sock;
    QString sockPath;
    QString selectedFilePath;
    QVector<QString> selectedFilePaths;
    int getState();
    QString sendRequest(char type, QString command);
public:
    MEGASyncPlugin(QObject* parent = 0, const QVariantList & args = QVariantList());
    virtual ~MEGASyncPlugin();
    virtual QList<QAction*> actions(const KFileItemListProperties & fileItemInfos, QWidget * parentWidget) override;

private slots:
    void getLink();
    void getLinks();
    void uploadFile();
    void uploadFiles();
    void viewOnMega();
    void viewPreviousVersions();
    QString getString(int type, int numFiles,int numFolders);

private:
    QAction* createChildAction(KActionMenu* menu, int type, int numFiles=0, int numFolders=0);
};

#endif

#ifndef MEGA_SYNC_PLUGIN_H
#define MEGA_SYNC_PLUGIN_H

#include <KAbstractFileItemActionPlugin>
#include <KActionMenu>
#include <KFileItem>
#include <KFileItemListProperties>
#include <KPluginFactory>

#include <QLocalSocket>

#include <kactionmenu.h>

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
    explicit MEGASyncPlugin(QObject* parent = nullptr, const QVariantList& args = QVariantList());
    ~MEGASyncPlugin() override;
    QList<QAction*> actions(const KFileItemListProperties& fileItemInfos,
                            QWidget* parentWidget) override;

private Q_SLOTS:
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

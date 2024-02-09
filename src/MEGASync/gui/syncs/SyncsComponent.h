#ifndef SYNCSCOMPONENT_H
#define SYNCSCOMPONENT_H

#include "qml/QmlDialogWrapper.h"

#include "syncs/control/SyncSettings.h"

class SyncsComponent : public QMLComponent
{
    Q_OBJECT

public:
    explicit SyncsComponent(QObject *parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    static void registerQmlModules();

    Q_INVOKABLE void openSyncsTabInPreferences() const;

    /*
private slots:
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);
*/

};

#endif // SYNCSCOMPONENT_H

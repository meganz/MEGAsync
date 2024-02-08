#ifndef SYNCSCOMPONENT_H
#define SYNCSCOMPONENT_H

#include "qml/QmlDialogWrapper.h"

class SyncsComponent : public QMLComponent
{
    Q_OBJECT

public:
    explicit SyncsComponent(QObject *parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    static void registerQmlModules();

    Q_INVOKABLE void openSyncsTabInPreferences() const;

};

#endif // SYNCSCOMPONENT_H

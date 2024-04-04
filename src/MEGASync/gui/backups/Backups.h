#ifndef BACKUPS_H
#define BACKUPS_H

#include "qml/QmlDialogWrapper.h"

class Backups : public QMLComponent
{
    Q_OBJECT

    Q_PROPERTY(bool comesFromSettings READ getComesFromSettings NOTIFY comesFromSettingsChanged)

public:
    explicit Backups(QObject *parent = 0);

    QUrl getQmlUrl() override;
    QString contextName() override;

    static void registerQmlModules();

    Q_INVOKABLE void openBackupsTabInPreferences() const;
    Q_INVOKABLE void openExclusionsDialog(const QString& folder) const;

    void setComesFromSettings(bool value = false);
    bool getComesFromSettings() const;

signals:
    void comesFromSettingsChanged(bool value);

private:
    bool mComesFromSettings;

};

#endif // BACKUPS_H

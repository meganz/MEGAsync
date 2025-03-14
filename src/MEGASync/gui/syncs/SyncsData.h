#ifndef SYNCS_DATA_H
#define SYNCS_DATA_H

#include <QObject>

class Syncs;
class SyncsData: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString defaultMegaFolder READ getDefaultMegaFolder CONSTANT FINAL)
    Q_PROPERTY(QString defaultMegaPath READ getDefaultMegaPath CONSTANT FINAL)
    Q_PROPERTY(QString localError READ getLocalError NOTIFY localErrorChanged)
    Q_PROPERTY(QString remoteError READ getRemoteError NOTIFY remoteErrorChanged)

    friend class Syncs;

public:
    explicit SyncsData(QObject* parent = nullptr);
    virtual ~SyncsData() = default;
    static QString getDefaultMegaFolder();
    static QString getDefaultMegaPath();

signals:
    void localErrorChanged();
    void remoteErrorChanged();
    void syncSetupSuccess();
    void syncRemoved();

private:
    QString getLocalError() const;
    QString getRemoteError() const;

    void setLocalError(const QString& error);
    void setRemoteError(const QString& error);

    QString mLocalError;
    QString mRemoteError;
};

#endif

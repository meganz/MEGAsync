#ifndef ENCRYPTEDSETTINGS_H
#define ENCRYPTEDSETTINGS_H

#include <QSettings>
#include <QVariant>
#include <QStringList>
#include <QCryptographicHash>

class EncryptedSettings : protected QSettings
{
    Q_OBJECT

public:
    explicit EncryptedSettings(QString file);

    void setValue(const QString & key, const QVariant & value);
    QVariant value(const QString & key, const QVariant & defaultValue = QVariant());
    void beginGroup(const QString & prefix);
    void endGroup();
    int numChildGroups();
    bool containsGroup(QString groupName);
    bool isGroupEmpty();
    void remove(const QString & key);
    void sync();

protected:
    QByteArray XOR(const QByteArray &key, const QByteArray& data) const;
    QString encrypt(const QString key, const QString value) const;
    QString decrypt(const QString key, const QString value) const;
    QString hash(const QString key) const;
    QByteArray encryptionKey;
};

#endif // ENCRYPTEDSETTINGS_H

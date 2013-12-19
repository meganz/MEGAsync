#ifndef ENCRYPTEDSETTINGS_H
#define ENCRYPTEDSETTINGS_H

#include <QSettings>
#include <QVariant>
#include <QStringList>

class EncryptedSettings : public QSettings
{
    Q_OBJECT

public:
    explicit EncryptedSettings(QByteArray encryptionKey, QString organizationName, QString applicationName);

    void setValue(const QString & key, const QVariant & value);
    QVariant value(const QString & key, const QVariant & defaultValue = QVariant() ) const;
    void beginGroup(const QString & prefix);
    QString	group() const;
    QStringList	childGroups () const;

protected:
    QByteArray XOR(const QByteArray& data) const;
    QString encrypt(const QString value) const;
    QString decrypt(const QString value) const;
    QByteArray encryptionKey;
};

#endif // ENCRYPTEDSETTINGS_H

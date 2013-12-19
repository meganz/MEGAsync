#include "EncryptedSettings.h"

EncryptedSettings::EncryptedSettings(QByteArray encryptionKey, QString organizationName, QString applicationName) :
    QSettings(organizationName, applicationName)
{
    this->encryptionKey = encryptionKey;
}

void EncryptedSettings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(encrypt(key), encrypt(value.toString()));
}

QVariant EncryptedSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return QVariant(decrypt(QSettings::value(encrypt(key), encrypt(defaultValue.toString())).toString()));
}

void EncryptedSettings::beginGroup(const QString &prefix)
{
    QSettings::beginGroup(encrypt(prefix));
}

QString EncryptedSettings::group() const
{
    return decrypt(QSettings::group());
}

QStringList EncryptedSettings::childGroups() const
{
    QStringList list = QSettings::childGroups();
    QStringList result;
    for(int i=0; i<list.size(); i++)
        result.append(decrypt(list[i]));
    return result;
}

QByteArray EncryptedSettings::XOR(const QByteArray& data) const
{
    QByteArray result;
    for(int i = 0 , j = 0; i < data.length(); ++i , ++j)
    {
        if(j == encryptionKey.length()) j = 0;
        result.append(data.at(i) ^ encryptionKey.at(j));
    }
    return result;
}

QString EncryptedSettings::encrypt(const QString value) const
{
    if(!value.size()) return value;
    return QString::fromAscii(qCompress(XOR(value.toUtf8().toHex())).toBase64().replace('/','-'));
}

QString EncryptedSettings::decrypt(const QString value) const
{
    if(!value.size()) return value;
    return QString::fromUtf8(QByteArray::fromHex(XOR(qUncompress(QByteArray::fromBase64(QString(value).replace('-','/').toAscii())))));
}

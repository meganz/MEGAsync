#ifndef REGISTRY_HIVE_READER_H
#define REGISTRY_HIVE_READER_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

// Read-only lookup of string values in a Windows registry hive file ("regf")
// held in memory.
//
// Exists for one caller: resolving the Snipping Tool's relocated capture folder
// on Windows. That folder is recorded in the app's private hive
// (Packages\...\SystemAppData\Helium\UserClasses.dat), which cannot be opened
// through the registry API from another process -- RegLoadAppKey rejects it
// and RegLoadKey needs SE_RESTORE_NAME -- so the file has to be read directly.
// Walking the key tree is the only reliable way to do that: the hive also
// stores a serialized shell link and MUI cache strings next to the entry we
// want, and a byte scan for "something that looks like a path" picked those up.
//
// Pure C++/Qt, no Win32 dependency, so it builds and is unit-tested on every
// platform. Everything is bounds-checked against the buffer: the file is
// written by another process and may be truncated or mid-rewrite when read.
// Anything unexpected yields an empty result rather than a guess.
//
// Only what the caller needs is implemented: nk/vk records, lf/lh/li/ri subkey
// lists, REG_SZ and REG_EXPAND_SZ data. Values stored in "db" big-data records
// (over 16 KB) are not supported and read as empty.
class RegistryHiveReader
{
public:
    explicit RegistryHiveReader(const QByteArray& hive);

    // True if the buffer carries a plausible hive header. The lookups below
    // already return empty on an invalid hive; this only lets callers log why.
    bool isValid() const;

    // Data of a REG_SZ / REG_EXPAND_SZ value, or empty if the key path, the
    // value or the hive itself is not as expected. Names compare
    // case-insensitively, as the registry does. keyPath is relative to the
    // hive's root key.
    QString readString(const QStringList& keyPath, const QString& valueName) const;

private:
    // A cell's payload (the bytes after its size field), as absolute offsets
    // into mHive. Produced only when the whole cell lies inside the buffer.
    struct Cell
    {
        int offset = 0;
        int size = 0;
    };

    bool cellAt(quint32 relativeOffset, Cell& cell) const;
    bool keyAt(quint32 relativeOffset, Cell& key) const;
    QString keyName(const Cell& key) const;
    quint32 findSubkey(const Cell& key, const QString& name) const;
    void collectSubkeys(quint32 listOffset, QVector<quint32>& subkeys, int depth) const;
    QString findStringValue(const Cell& key, const QString& name) const;

    quint32 u32At(int absoluteOffset) const;
    quint16 u16At(int absoluteOffset) const;
    QString nameAt(int absoluteOffset, int length, bool compressed) const;

    QByteArray mHive;
    quint32 mRootKeyOffset = 0;
    bool mValid = false;
};

#endif // REGISTRY_HIVE_READER_H

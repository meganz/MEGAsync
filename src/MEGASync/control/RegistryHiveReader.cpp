#include "RegistryHiveReader.h"

#include <QtEndian>

// Layout per the public descriptions of the hive format (Windows NT 4 through
// Windows 11 share it). Offsets are into a cell's payload unless stated.
namespace
{
constexpr int HEADER_SIZE = 0x1000; // cell offsets are relative to its end
constexpr int HEADER_ROOT_CELL_OFFSET = 0x24;
const QByteArray HIVE_SIGNATURE("regf");

// Key node ("nk").
constexpr int NK_FLAGS = 0x02;
constexpr int NK_SUBKEY_COUNT = 0x14;
constexpr int NK_SUBKEY_LIST = 0x1c;
constexpr int NK_VALUE_COUNT = 0x24;
constexpr int NK_VALUE_LIST = 0x28;
constexpr int NK_NAME_LENGTH = 0x48;
constexpr int NK_NAME = 0x4c;
constexpr quint16 NK_FLAG_COMPRESSED_NAME = 0x0020;

// Value ("vk").
constexpr int VK_NAME_LENGTH = 0x02;
constexpr int VK_DATA_LENGTH = 0x04;
constexpr int VK_DATA_OFFSET = 0x08;
constexpr int VK_TYPE = 0x0c;
constexpr int VK_FLAGS = 0x10;
constexpr int VK_NAME = 0x14;
constexpr quint16 VK_FLAG_COMPRESSED_NAME = 0x0001;
constexpr quint32 VK_DATA_INLINE = 0x80000000u; // data lives in the offset field
constexpr quint32 VK_DATA_MAX_INLINE = 4;
constexpr quint32 VK_DATA_MAX_NORMAL = 16344; // larger values use "db" records
constexpr quint32 REG_SZ_TYPE = 1;
constexpr quint32 REG_EXPAND_SZ_TYPE = 2;

// Subkey lists: a count followed by entries. lf/lh pair each offset with a hash,
// li holds bare offsets, ri holds offsets to further lists.
constexpr int LIST_COUNT = 0x02;
constexpr int LIST_ENTRIES = 0x04;
constexpr int MAX_LIST_NESTING = 4;

constexpr quint32 INVALID_OFFSET = 0xffffffffu;
}

RegistryHiveReader::RegistryHiveReader(const QByteArray& hive):
    mHive(hive)
{
    if (mHive.size() < HEADER_SIZE || !mHive.startsWith(HIVE_SIGNATURE))
    {
        return;
    }

    mRootKeyOffset = u32At(HEADER_ROOT_CELL_OFFSET);
    Cell root;
    mValid = keyAt(mRootKeyOffset, root);
}

bool RegistryHiveReader::isValid() const
{
    return mValid;
}

QString RegistryHiveReader::readString(const QStringList& keyPath, const QString& valueName) const
{
    if (!mValid)
    {
        return QString();
    }

    quint32 keyOffset = mRootKeyOffset;
    Cell key;
    for (const QString& name: keyPath)
    {
        if (!keyAt(keyOffset, key))
        {
            return QString();
        }
        keyOffset = findSubkey(key, name);
        if (keyOffset == INVALID_OFFSET)
        {
            return QString();
        }
    }

    if (!keyAt(keyOffset, key))
    {
        return QString();
    }
    return findStringValue(key, valueName);
}

bool RegistryHiveReader::cellAt(quint32 relativeOffset, Cell& cell) const
{
    // Guard the additions themselves: offsets come from the file.
    if (relativeOffset == INVALID_OFFSET ||
        relativeOffset > static_cast<quint32>(mHive.size() - HEADER_SIZE))
    {
        return false;
    }

    const int sizeFieldOffset = HEADER_SIZE + static_cast<int>(relativeOffset);
    if (sizeFieldOffset + static_cast<int>(sizeof(qint32)) > mHive.size())
    {
        return false;
    }

    // Negative size marks an allocated cell; the magnitude includes the size
    // field itself. A positive size is free space, never valid data.
    const qint32 rawSize = static_cast<qint32>(u32At(sizeFieldOffset));
    if (rawSize >= -static_cast<qint32>(sizeof(qint32)))
    {
        return false;
    }
    const qint64 payloadSize = -static_cast<qint64>(rawSize) - static_cast<qint64>(sizeof(qint32));
    const qint64 payloadOffset = sizeFieldOffset + static_cast<qint64>(sizeof(qint32));
    if (payloadOffset + payloadSize > mHive.size())
    {
        return false;
    }

    cell.offset = static_cast<int>(payloadOffset);
    cell.size = static_cast<int>(payloadSize);
    return true;
}

bool RegistryHiveReader::keyAt(quint32 relativeOffset, Cell& key) const
{
    if (!cellAt(relativeOffset, key) || key.size < NK_NAME)
    {
        return false;
    }
    return mHive[key.offset] == 'n' && mHive[key.offset + 1] == 'k';
}

QString RegistryHiveReader::keyName(const Cell& key) const
{
    const int length = u16At(key.offset + NK_NAME_LENGTH);
    if (NK_NAME + length > key.size)
    {
        return QString();
    }
    const bool compressed = (u16At(key.offset + NK_FLAGS) & NK_FLAG_COMPRESSED_NAME) != 0;
    return nameAt(key.offset + NK_NAME, length, compressed);
}

quint32 RegistryHiveReader::findSubkey(const Cell& key, const QString& name) const
{
    const quint32 count = u32At(key.offset + NK_SUBKEY_COUNT);
    if (count == 0)
    {
        return INVALID_OFFSET;
    }

    QVector<quint32> subkeys;
    collectSubkeys(u32At(key.offset + NK_SUBKEY_LIST), subkeys, 0);

    for (const quint32 subkeyOffset: subkeys)
    {
        Cell subkey;
        if (keyAt(subkeyOffset, subkey) && keyName(subkey).compare(name, Qt::CaseInsensitive) == 0)
        {
            return subkeyOffset;
        }
    }
    return INVALID_OFFSET;
}

void RegistryHiveReader::collectSubkeys(quint32 listOffset,
                                        QVector<quint32>& subkeys,
                                        int depth) const
{
    Cell list;
    if (depth > MAX_LIST_NESTING || !cellAt(listOffset, list) || list.size < LIST_ENTRIES)
    {
        return;
    }

    const char first = mHive[list.offset];
    const char second = mHive[list.offset + 1];
    const bool hashed = first == 'l' && (second == 'f' || second == 'h');
    const bool plain = first == 'l' && second == 'i';
    const bool indirect = first == 'r' && second == 'i';
    if (!hashed && !plain && !indirect)
    {
        return;
    }

    const int entrySize =
        hashed ? 2 * static_cast<int>(sizeof(quint32)) : static_cast<int>(sizeof(quint32));
    const int count = u16At(list.offset + LIST_COUNT);
    if (LIST_ENTRIES + count * entrySize > list.size)
    {
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        const quint32 entry = u32At(list.offset + LIST_ENTRIES + i * entrySize);
        if (indirect)
        {
            collectSubkeys(entry, subkeys, depth + 1);
        }
        else
        {
            subkeys.append(entry);
        }
    }
}

QString RegistryHiveReader::findStringValue(const Cell& key, const QString& name) const
{
    const quint32 count = u32At(key.offset + NK_VALUE_COUNT);
    Cell list;
    if (count == 0 || !cellAt(u32At(key.offset + NK_VALUE_LIST), list) ||
        static_cast<qint64>(count) * static_cast<qint64>(sizeof(quint32)) > list.size)
    {
        return QString();
    }

    for (quint32 i = 0; i < count; ++i)
    {
        Cell value;
        if (!cellAt(u32At(list.offset + static_cast<int>(i * sizeof(quint32))), value) ||
            value.size < VK_NAME || mHive[value.offset] != 'v' || mHive[value.offset + 1] != 'k')
        {
            continue;
        }

        const int nameLength = u16At(value.offset + VK_NAME_LENGTH);
        if (VK_NAME + nameLength > value.size)
        {
            continue;
        }
        const bool compressed = (u16At(value.offset + VK_FLAGS) & VK_FLAG_COMPRESSED_NAME) != 0;
        if (nameAt(value.offset + VK_NAME, nameLength, compressed)
                .compare(name, Qt::CaseInsensitive) != 0)
        {
            continue;
        }

        const quint32 type = u32At(value.offset + VK_TYPE);
        if (type != REG_SZ_TYPE && type != REG_EXPAND_SZ_TYPE)
        {
            return QString();
        }

        const quint32 rawLength = u32At(value.offset + VK_DATA_LENGTH);
        const quint32 length = rawLength & ~VK_DATA_INLINE;
        const char* data = nullptr;
        if (rawLength & VK_DATA_INLINE)
        {
            if (length > VK_DATA_MAX_INLINE)
            {
                return QString();
            }
            data = mHive.constData() + value.offset + VK_DATA_OFFSET;
        }
        else
        {
            Cell dataCell;
            if (length > VK_DATA_MAX_NORMAL ||
                !cellAt(u32At(value.offset + VK_DATA_OFFSET), dataCell) ||
                static_cast<int>(length) > dataCell.size)
            {
                return QString();
            }
            data = mHive.constData() + dataCell.offset;
        }

        // UTF-16LE, usually with a terminating NUL that is part of the length.
        QString text = QString::fromUtf16(reinterpret_cast<const char16_t*>(data),
                                          static_cast<int>(length / sizeof(char16_t)));
        while (text.endsWith(QChar::Null))
        {
            text.chop(1);
        }
        return text;
    }

    return QString();
}

quint32 RegistryHiveReader::u32At(int absoluteOffset) const
{
    if (absoluteOffset < 0 || absoluteOffset + static_cast<int>(sizeof(quint32)) > mHive.size())
    {
        return INVALID_OFFSET;
    }
    return qFromLittleEndian<quint32>(mHive.constData() + absoluteOffset);
}

quint16 RegistryHiveReader::u16At(int absoluteOffset) const
{
    if (absoluteOffset < 0 || absoluteOffset + static_cast<int>(sizeof(quint16)) > mHive.size())
    {
        return 0xffff;
    }
    return qFromLittleEndian<quint16>(mHive.constData() + absoluteOffset);
}

QString RegistryHiveReader::nameAt(int absoluteOffset, int length, bool compressed) const
{
    if (absoluteOffset < 0 || length < 0 || absoluteOffset + length > mHive.size())
    {
        return QString();
    }
    const char* bytes = mHive.constData() + absoluteOffset;
    // "Compressed" names are stored one byte per character (Latin-1), the
    // rest as UTF-16LE.
    return compressed ? QString::fromLatin1(bytes, length) :
                        QString::fromUtf16(reinterpret_cast<const char16_t*>(bytes),
                                           length / static_cast<int>(sizeof(char16_t)));
}

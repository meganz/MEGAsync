#include "RegistryHiveReader.h"
#include <catch.hpp>

#include <QFile>

// The fixture is the Snipping Tool's private hive from a Windows 11 machine
// (Packages\Microsoft.ScreenSketch_8wekyb3d8bbwe\SystemAppData\Helium\
// UserClasses.dat) after the capture folder was relocated twice. It holds two
// FutureAccessList entries, one per folder ever chosen, keyed by the token the
// Snipping Tool stores in its settings. These tests pin the lookup the Windows
// platform layer relies on, and that the reader stays inside the buffer
// whatever it is handed: the file belongs to another process and may be
// truncated or mid-rewrite when read.

namespace
{

const QStringList MANAGED_BY_APP{QString::fromUtf8("Local Settings"),
                                 QString::fromUtf8("Software"),
                                 QString::fromUtf8("Microsoft"),
                                 QString::fromUtf8("Windows"),
                                 QString::fromUtf8("CurrentVersion"),
                                 QString::fromUtf8("AppModel"),
                                 QString::fromUtf8("SystemAppData"),
                                 QString::fromUtf8("Microsoft.ScreenSketch_8wekyb3d8bbwe"),
                                 QString::fromUtf8("PersistedStorageItemTable"),
                                 QString::fromUtf8("ManagedByApp")};

const char* const FIRST_TOKEN = "{6D6C522E-2F56-415E-9FC4-7AC2BDD7EE6C}";
const char* const SECOND_TOKEN = "{AD469B09-92F7-4C6E-8047-EBA464162545}";
const char* const METADATA = "Metadata";
const char* const FILE_PATH = "FilePath";

// Offset past the whole first entry (key, values and data) and before the
// second one, taken from the fixture's layout.
constexpr int BETWEEN_ENTRIES = 0x2600;

QByteArray fixtureHive()
{
    QFile file(QString::fromUtf8(UNIT_TESTS_DATA_DIR "/ScreenSketchUserClasses.dat"));
    REQUIRE(file.open(QIODevice::ReadOnly));
    return file.readAll();
}

QStringList entry(const char* token)
{
    return MANAGED_BY_APP + QStringList{QString::fromUtf8(token)};
}

std::string read(const RegistryHiveReader& reader, const QStringList& keyPath, const char* value)
{
    return reader.readString(keyPath, QString::fromUtf8(value)).toStdString();
}

} // namespace

TEST_CASE("RegistryHiveReader reads the Snipping Tool capture folder entries")
{
    const RegistryHiveReader reader(fixtureHive());
    REQUIRE(reader.isValid());

    // The plain path is what the detector watches; the volume-GUID form is the
    // fallback.
    CHECK(read(reader, entry(FIRST_TOKEN), METADATA) == "C:\\Users\\mega\\Desktop\\0_temp_scr");
    CHECK(read(reader, entry(FIRST_TOKEN), FILE_PATH) ==
          "\\\\?\\Volume{99156CD4-6BD2-4376-9114-504ED7BA736F}\\Users\\mega\\Desktop\\0_temp_scr");
    CHECK(read(reader, entry(SECOND_TOKEN), METADATA) == "C:\\Users\\mega\\Desktop\\0_temp4");
    CHECK(read(reader, entry(SECOND_TOKEN), FILE_PATH) ==
          "\\\\?\\Volume{99156CD4-6BD2-4376-9114-504ED7BA736F}\\Users\\mega\\Desktop\\0_temp4");
}

TEST_CASE("RegistryHiveReader answers empty for what is not a string value")
{
    const RegistryHiveReader reader(fixtureHive());

    // Present, but REG_BINARY and REG_DWORD: never interpreted as text.
    CHECK(read(reader, entry(FIRST_TOKEN), "LastUpdatedTime").empty());
    CHECK(read(reader, entry(FIRST_TOKEN), "Flags").empty());
    CHECK(read(reader, entry(FIRST_TOKEN), "Link").empty());

    // Missing value, missing token, path that stops short of a leaf.
    CHECK(read(reader, entry(FIRST_TOKEN), "NoSuchValue").empty());
    CHECK(read(reader, entry("{00000000-0000-0000-0000-000000000000}"), METADATA).empty());
    CHECK(read(reader, MANAGED_BY_APP, METADATA).empty());
    CHECK(read(reader, QStringList(), METADATA).empty());
}

TEST_CASE("RegistryHiveReader compares key and value names case-insensitively")
{
    const RegistryHiveReader reader(fixtureHive());

    QStringList shouted;
    for (const QString& name: entry(FIRST_TOKEN))
    {
        shouted << name.toUpper();
    }
    CHECK(read(reader, shouted, "METADATA") == "C:\\Users\\mega\\Desktop\\0_temp_scr");
    CHECK(read(reader, shouted, "metadata") == "C:\\Users\\mega\\Desktop\\0_temp_scr");
}

TEST_CASE("RegistryHiveReader rejects malformed input without leaving the buffer")
{
    const QByteArray hive = fixtureHive();

    SECTION("not a hive at all")
    {
        CHECK_FALSE(RegistryHiveReader(QByteArray()).isValid());
        CHECK_FALSE(RegistryHiveReader(QByteArray("regf")).isValid());
        CHECK_FALSE(RegistryHiveReader(QByteArray(hive.size(), '\0')).isValid());
        CHECK(read(RegistryHiveReader(QByteArray(hive.size(), '\0')), entry(FIRST_TOKEN), METADATA)
                  .empty());
    }

    SECTION("header only, no cells")
    {
        const RegistryHiveReader reader(hive.left(0x1000));
        CHECK_FALSE(reader.isValid());
        CHECK(read(reader, entry(FIRST_TOKEN), METADATA).empty());
    }

    SECTION("truncated at every page boundary and at odd sizes")
    {
        for (const int size:
             {0x1001, 0x1020, 0x1800, 0x1d00, 0x1d70, 0x1e00, 0x2000, 0x2480, 0x3000})
        {
            INFO("truncated to " << size << " bytes");
            const RegistryHiveReader reader(hive.left(size));
            // Must not crash or read past the end; the result may be empty or,
            // once the whole entry fits, the real value.
            const std::string value = read(reader, entry(FIRST_TOKEN), METADATA);
            CHECK((value.empty() || value == "C:\\Users\\mega\\Desktop\\0_temp_scr"));
        }
    }

    SECTION("a cut between the two entries keeps the first one readable")
    {
        const RegistryHiveReader reader(hive.left(BETWEEN_ENTRIES));
        CHECK(read(reader, entry(FIRST_TOKEN), METADATA) == "C:\\Users\\mega\\Desktop\\0_temp_scr");
        CHECK(read(reader, entry(SECOND_TOKEN), METADATA).empty());
    }

    SECTION("root offset pointing outside the file")
    {
        QByteArray corrupted = hive;
        corrupted[0x24] = static_cast<char>(0xff);
        corrupted[0x25] = static_cast<char>(0xff);
        corrupted[0x26] = static_cast<char>(0xff);
        corrupted[0x27] = static_cast<char>(0x7f);
        const RegistryHiveReader reader(corrupted);
        CHECK_FALSE(reader.isValid());
        CHECK(read(reader, entry(FIRST_TOKEN), METADATA).empty());
    }

    SECTION("every byte flipped one at a time in the header and first cells")
    {
        for (int i = 0; i < 0x1100; ++i)
        {
            QByteArray corrupted = hive;
            corrupted[i] = static_cast<char>(~corrupted[i]);
            const RegistryHiveReader reader(corrupted);
            read(reader, entry(FIRST_TOKEN), METADATA); // only the absence of a crash matters
        }
        SUCCEED();
    }
}

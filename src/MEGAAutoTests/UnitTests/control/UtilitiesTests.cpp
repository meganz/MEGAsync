#include "Platform.h"
#include "TestSingletons.h"
#include "Utilities.h"
#include <catch.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

class UtilitiesTests
{
protected:
    void testGetSizeString(unsigned long long value, const std::string& expected)
    {
        CHECK(Utilities::getSizeString(value).toStdString() == expected);
    }
};

TEST_CASE_METHOD(UtilitiesTests, "getQuantityString()")
{
    testGetSizeString(1, "1 Bytes");
    testGetSizeString(10, "10 Bytes");
    testGetSizeString(100, "100 Bytes");
    testGetSizeString(1023, "1023 Bytes");

    testGetSizeString(1024, "1 KB");
    testGetSizeString(1025, "1 KB");
    testGetSizeString(1532, "1.5 KB");
    testGetSizeString(10981, "10.72 KB");
    testGetSizeString(10949, "10.69 KB");
    testGetSizeString(15391, "15.03 KB");
    testGetSizeString(198741, "194.08 KB");

    testGetSizeString(1024 * 1024, "1 MB");
    testGetSizeString(1532 * 1024, "1.5 MB");
    testGetSizeString(15391 * 1024, "15.03 MB");
    testGetSizeString(198741 * 1024, "194.08 MB");

    testGetSizeString(1024 * 1024 * 1024LL, "1 GB");
    testGetSizeString(1532 * 1024 * 1024LL, "1.5 GB");
    testGetSizeString(15391 * 1024 * 1024LL, "15.03 GB");
    testGetSizeString(198741 * 1024 * 1024LL, "194.08 GB");

    testGetSizeString(1024 * 1024 * 1024LL * 1024LL, "1 TB");
    testGetSizeString(1532 * 1024 * 1024LL * 1024LL, "1.5 TB");
    testGetSizeString(15391 * 1024 * 1024LL * 1024LL, "15.03 TB");
    testGetSizeString(198741 * 1024 * 1024LL * 1024LL, "194.08 TB");
}

TEST_CASE_METHOD(UtilitiesTests, "getTimeString()")
{
    // "Changes on time format" (commit 4452c4481) dropped the HTML styling that
    // used to wrap the unit letters in a <span>; getTimeString now returns plain
    // text. The leading unit is space-padded to two chars, the rest zero-padded.
    constexpr auto daySeconds{86400};
    constexpr auto hourSeconds{3600};
    constexpr auto minuteSeconds{60};

    REQUIRE(Utilities::getTimeString((daySeconds * 3) + (13 * hourSeconds)).toStdString() ==
            " 3d 13h");

    REQUIRE(Utilities::getTimeString(hourSeconds + (5 * minuteSeconds)).toStdString() == " 1h 05m");

    REQUIRE(Utilities::getTimeString((5 * minuteSeconds) + 7).toStdString() == " 5m 07s");

    REQUIRE(Utilities::getTimeString((10 * minuteSeconds) + 7).toStdString() == "10m 07s");

    constexpr auto secondsPrecision{false};
    REQUIRE(Utilities::getTimeString((5 * minuteSeconds) + 7, secondsPrecision).toStdString() ==
            " 5m");
}

// restrictFolderToOwner() only reports what it had to change, so the recorded messages are
// what tells its outcomes apart: the folder ends up restricted either way
class FolderPermissionsTests
{
public:
    FolderPermissionsTests()
    {
        ensurePlatformCreated();
        REQUIRE(mTempDir.isValid());
    }

protected:
    Utilities::PermissionsLogger recorder()
    {
        return [this](int logLevel, const QString& message)
        {
            mLogLevels.push_back(logLevel);
            mMessages.push_back(message);
        };
    }

    QString createFolder(const QString& name) const
    {
        QDir dir(mTempDir.path());
        REQUIRE(dir.mkpath(name));
        return dir.absoluteFilePath(name);
    }

    QString path(const QString& name) const
    {
        return mTempDir.filePath(name);
    }

    void checkNothingLogged() const
    {
        CHECK(mMessages.empty());
    }

    void checkLoggedOnce(int expectedLogLevel, const QString& expectedText) const
    {
        REQUIRE(mMessages.size() == 1);
        CHECK(mLogLevels.front() == expectedLogLevel);
        CHECK(mMessages.front().contains(expectedText));
    }

private:
    QTemporaryDir mTempDir;
    std::vector<int> mLogLevels;
    std::vector<QString> mMessages;
};

TEST_CASE_METHOD(FolderPermissionsTests,
                 "restrictFolderToOwner() stays quiet on a restricted folder")
{
    const QString folderPath(path(QLatin1String("restricted")));
    REQUIRE(Platform::getInstance()->createFolderRestrictedToOwner(folderPath, nullptr));

    Utilities::restrictFolderToOwner(folderPath, recorder());

    // Nothing had to change, so a normal start must not log anything at all
    checkNothingLogged();
    CHECK(Platform::getInstance()->isRestrictedToOwner(folderPath));
}

TEST_CASE_METHOD(FolderPermissionsTests, "restrictFolderToOwner() reports a missing folder")
{
    const QString folderPath(path(QLatin1String("never-created")));

    Utilities::restrictFolderToOwner(folderPath, recorder());

    // Creating the folder failing earlier must not pass silently: nothing is protected
    checkLoggedOnce(mega::MegaApi::LOG_LEVEL_ERROR, folderPath);
}

TEST_CASE_METHOD(FolderPermissionsTests,
                 "restrictFolderToOwner() reports a path that is not a folder")
{
    const QString filePath(path(QLatin1String("a-file")));
    QFile file(filePath);
    REQUIRE(file.open(QIODevice::WriteOnly));
    file.close();

    Utilities::restrictFolderToOwner(filePath, recorder());

    checkLoggedOnce(mega::MegaApi::LOG_LEVEL_ERROR, filePath);
}

// isLocalFolderWritable() probes a folder by creating a file in it, so every case here is
// checked against a real directory rather than against the permissions the filesystem reports
class LocalFolderWritableTests
{
public:
    LocalFolderWritableTests()
    {
        REQUIRE(mTempDir.isValid());
    }

protected:
    QString root() const
    {
        return mTempDir.path();
    }

    QString path(const QString& name) const
    {
        return mTempDir.filePath(name);
    }

    int entryCount() const
    {
        return QDir(mTempDir.path())
            .entryList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot)
            .size();
    }

private:
    QTemporaryDir mTempDir;
};

TEST_CASE_METHOD(LocalFolderWritableTests, "isLocalFolderWritable() accepts a writable folder")
{
    CHECK(Utilities::isLocalFolderWritable(root()));
}

TEST_CASE_METHOD(LocalFolderWritableTests,
                 "isLocalFolderWritable() accepts the separators the dialogs hand it")
{
    // The download dialogs keep their line edit in native separators and the folder selector
    // returns '/' ones, so both spellings reach the check, with or without a trailing separator
    CHECK(Utilities::isLocalFolderWritable(QDir::toNativeSeparators(root())));
    CHECK(Utilities::isLocalFolderWritable(root() + QLatin1Char('/')));
    CHECK(Utilities::isLocalFolderWritable(QDir::toNativeSeparators(root()) + QDir::separator()));
}

TEST_CASE_METHOD(LocalFolderWritableTests, "isLocalFolderWritable() rejects an empty path")
{
    CHECK_FALSE(Utilities::isLocalFolderWritable(QString()));
}

TEST_CASE_METHOD(LocalFolderWritableTests, "isLocalFolderWritable() rejects a missing folder")
{
    CHECK_FALSE(Utilities::isLocalFolderWritable(path(QLatin1String("never-created"))));
}

TEST_CASE_METHOD(LocalFolderWritableTests, "isLocalFolderWritable() leaves nothing behind")
{
    const int before(entryCount());

    REQUIRE(Utilities::isLocalFolderWritable(root()));

    // The probe file is a QTemporaryFile, so a folder the user picked must come out untouched
    CHECK(entryCount() == before);
}

#ifdef Q_OS_WINDOWS
TEST_CASE_METHOD(LocalFolderWritableTests,
                 "isLocalFolderWritable() accepts a path starting with a double backslash")
{
    // SNC-6919: QTemporaryFile resolves its template through QFileSystemEntry, which only
    // treats "X:" or a leading '/' as absolute. Any native path starting with "\\" therefore
    // looks relative to it and gets joined onto the working directory, which is what made a
    // network share ("\\machine\folder") report itself as not writable. The extended-length
    // prefix trips the very same check, so it pins the regression without needing a share.
    const QString extendedLengthPath(QLatin1String("\\\\?\\") + QDir::toNativeSeparators(root()));

    CHECK(Utilities::isLocalFolderWritable(extendedLengthPath));
}
#endif

#ifndef Q_OS_WINDOWS
// Exposing a folder to other users takes POSIX permission bits, which is also the only
// place where restrictFolderToOwner() has anything to do: on Windows the per-user location
// is already restricted through the ACLs it inherits
TEST_CASE_METHOD(FolderPermissionsTests,
                 "restrictFolderToOwner() closes a folder open to other users")
{
    const QString folderPath(createFolder(QLatin1String("exposed")));
    REQUIRE(QFile::setPermissions(folderPath,
                                  QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                      QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther |
                                      QFile::ExeOther));

    Utilities::restrictFolderToOwner(folderPath, recorder());

    CHECK(Platform::getInstance()->isRestrictedToOwner(folderPath));
    // Reported as info, and worth naming the mode it was left in by whoever created it
    checkLoggedOnce(mega::MegaApi::LOG_LEVEL_INFO, QLatin1String("0755"));
}

TEST_CASE_METHOD(FolderPermissionsTests, "restrictFolderToOwner() names the folder it changed")
{
    const QString folderPath(createFolder(QLatin1String("named")));
    REQUIRE(QFile::setPermissions(folderPath,
                                  QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                      QFile::ReadOther));

    Utilities::restrictFolderToOwner(folderPath, recorder());

    checkLoggedOnce(mega::MegaApi::LOG_LEVEL_INFO, folderPath);
    CHECK(Platform::getInstance()->isRestrictedToOwner(folderPath));
}

TEST_CASE_METHOD(FolderPermissionsTests, "restrictFolderToOwner() is applied on every call")
{
    const QString folderPath(createFolder(QLatin1String("loosened")));

    // A folder loosened after a previous start has to be tightened again, which is why the
    // permissions are enforced on every start and not only when the folder is created
    for (int pass = 0; pass < 2; ++pass)
    {
        REQUIRE(QFile::setPermissions(folderPath,
                                      QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                          QFile::ReadGroup | QFile::ExeGroup));

        Utilities::restrictFolderToOwner(folderPath, [](int, const QString&) {});

        CHECK(Platform::getInstance()->isRestrictedToOwner(folderPath));
    }
}
#endif

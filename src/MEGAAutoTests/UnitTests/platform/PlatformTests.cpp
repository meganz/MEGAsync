#include "Platform.h"
#include "TestSingletons.h"
#ifdef Q_OS_WINDOWS
#include "WindowsFileAttributePath.h"
#endif
#include <catch.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

class PlatformTests
{
public:
    PlatformTests()
    {
        ensurePlatformCreated();
        REQUIRE(mTempDir.isValid());
    }

protected:
    QString createFile(const QString& name) const
    {
        const QString filePath(path(name));
        QFile file(filePath);
        REQUIRE(file.open(QIODevice::WriteOnly));
        file.close();
        return filePath;
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

    QString tempPath() const
    {
        return mTempDir.path();
    }

    // Only Windows stores a Hidden attribute, elsewhere hiding is a naming convention and
    // setHidden() has nothing to do
    static void checkIsHidden(const QString& itemPath)
    {
#ifdef Q_OS_WINDOWS
        CHECK(QFileInfo(itemPath).isHidden());
#else
        CHECK(QFileInfo::exists(itemPath));
#endif
    }

private:
    QTemporaryDir mTempDir;
};

TEST_CASE_METHOD(PlatformTests, "setHidden() on a file")
{
    const auto filePath(createFile(QLatin1String("visible.txt")));

    CHECK(Platform::getInstance()->setHidden(filePath));

    // Hiding must not remove or replace the item
    CHECK(QFileInfo::exists(filePath));
    checkIsHidden(filePath);
}

TEST_CASE_METHOD(PlatformTests, "setHidden() on a folder")
{
    const auto folderPath(createFolder(QLatin1String("subfolder")));

    CHECK(Platform::getInstance()->setHidden(folderPath));

    CHECK(QFileInfo(folderPath).isDir());
    checkIsHidden(folderPath);
}

TEST_CASE_METHOD(PlatformTests, "setHidden() called twice")
{
    const auto filePath(createFile(QLatin1String("twice.txt")));

    CHECK(Platform::getInstance()->setHidden(filePath));

    // An already hidden item must report success instead of failing on the second call
    CHECK(Platform::getInstance()->setHidden(filePath));
    checkIsHidden(filePath);
}

TEST_CASE_METHOD(PlatformTests, "setHidden() keeps the other attributes")
{
    const auto filePath(createFile(QLatin1String("readonly.txt")));

    QFile file(filePath);
    REQUIRE(file.setPermissions(QFileDevice::ReadOwner));
    const bool wasWritable(QFileInfo(filePath).isWritable());

    CHECK(Platform::getInstance()->setHidden(filePath));

    // The attributes are combined, not replaced, so the read only state must be untouched
    CHECK(QFileInfo(filePath).isWritable() == wasWritable);
    CHECK(QFileInfo(filePath).isReadable());
    checkIsHidden(filePath);

    // Restore the permissions so QTemporaryDir can clean up
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

TEST_CASE_METHOD(PlatformTests, "setHidden() on a path that does not exist")
{
    const auto missingPath(path(QLatin1String("does_not_exist.txt")));
    REQUIRE_FALSE(QFileInfo::exists(missingPath));

#ifdef Q_OS_WINDOWS
    // The attribute cannot be applied, and callers rely on being told
    CHECK_FALSE(Platform::getInstance()->setHidden(missingPath));
#else
    // There is nothing to do on these platforms, so there is nothing that can fail
    CHECK(Platform::getInstance()->setHidden(missingPath));
#endif
}

TEST_CASE_METHOD(PlatformTests, "setHidden() on a long path with . and .. components")
{
    // Windows needs the \\?\ extended length prefix past its 260 character
    // MAX_PATH
    constexpr int MAX_PATH_LENGTH = 260;

    QDir dir(tempPath());
    const QString longComponent(80, QLatin1Char('d'));
    QString relativePath;
    while (dir.absolutePath().size() + relativePath.size() < MAX_PATH_LENGTH + 40)
    {
        relativePath += longComponent + QLatin1Char('/');
    }

    if (!dir.mkpath(relativePath + QLatin1String("parent")))
    {
        SKIP("The file system does not accept paths longer than MAX_PATH");
    }

    const QString filePath(dir.absoluteFilePath(relativePath + QLatin1String("deep.txt")));
    REQUIRE(filePath.size() > MAX_PATH_LENGTH);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        SKIP("The file system does not accept paths longer than MAX_PATH");
    }
    file.close();

    // Extended-length paths are not normalized by Win32, so exercise both code
    // paths together.
    const QString indirectPath(
        dir.absoluteFilePath(relativePath + QLatin1String("parent/../deep.txt")));
    REQUIRE(indirectPath.size() > MAX_PATH_LENGTH);

    CHECK(Platform::getInstance()->setHidden(indirectPath));
    checkIsHidden(filePath);

#ifdef Q_OS_WINDOWS
    QString extendedIndirectPath(QDir::toNativeSeparators(indirectPath));
    extendedIndirectPath.prepend(QLatin1String("\\\\?\\"));
    CHECK(Platform::getInstance()->setHidden(extendedIndirectPath));
#endif
}

TEST_CASE_METHOD(PlatformTests, "setHidden() on a path with . and .. components")
{
    createFolder(QLatin1String("parent"));
    const auto filePath(createFile(QLatin1String("target.txt")));

    // Windows does not resolve "." and ".." on extended length paths, so they have to be
    // normalised before the path is handed to the OS
    const QString indirectPath(path(QLatin1String("parent")) + QLatin1String("/../target.txt"));

    CHECK(Platform::getInstance()->setHidden(indirectPath));
    checkIsHidden(filePath);
}

#ifdef Q_OS_WINDOWS
TEST_CASE("Windows file-attribute paths preserve an extended prefix while cleaning")
{
    const QString longFolder(240, QLatin1Char('d'));
    const QString input(QLatin1String("\\\\?\\C:\\root\\") + longFolder +
                        QLatin1String("\\parent\\..\\file"));
    const QString expected(QLatin1String("\\\\?\\C:\\root\\") + longFolder +
                           QLatin1String("\\file"));

    CHECK(WindowsFileAttributePath::prepare(input) == expected);
}

TEST_CASE("Windows file-attribute paths convert long UNC paths")
{
    const QString longFolder(240, QLatin1Char('d'));
    const QString input(QLatin1String("\\\\server\\share\\") + longFolder +
                        QLatin1String("\\parent\\..\\file"));
    const QString expected(QLatin1String("\\\\?\\UNC\\server\\share\\") + longFolder +
                           QLatin1String("\\file"));

    CHECK(WindowsFileAttributePath::prepare(input) == expected);
}
#endif

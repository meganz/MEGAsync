#include "MegaIgnoreManager.h"
#include "Platform.h"
#include "TestSingletons.h"
#include <catch.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTextStream>

namespace
{
constexpr char DEFAULT_RULES[] = "-:*.tmp\n-:*.log\n";
}

class MegaIgnoreManagerTests
{
public:
    MegaIgnoreManagerTests()
    {
        ensurePlatformCreated();
        REQUIRE(mDataDir.isValid());
        REQUIRE(mSyncDir.isValid());
    }

protected:
    QString defaultFilePath() const
    {
        return mDataDir.filePath(
            QString::fromUtf8(MegaIgnoreManager::MEGA_IGNORE_DEFAULT_FILE_NAME));
    }

    QString outputFilePath() const
    {
        return mSyncDir.filePath(QString::fromUtf8(MegaIgnoreManager::MEGA_IGNORE_FILE_NAME));
    }

    void writeFile(const QString& filePath, const QString& contents) const
    {
        QFile file(filePath);
        REQUIRE(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // TODO QT6
        out.setEncoding(QStringConverter::Utf8);
#else
        out.setCodec("UTF-8");
#endif
        out << contents;
        file.close();
    }

    QString readFile(const QString& filePath) const
    {
        QFile file(filePath);
        REQUIRE(file.open(QIODevice::ReadOnly));
        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // TODO QT6
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        return in.readAll();
    }

    void writeDefaultFile() const
    {
        writeFile(defaultFilePath(), QString::fromUtf8(DEFAULT_RULES));
    }

    // restoreDefaults() only needs the default and output paths, so the default
    // constructor keeps the test clear of the parsing done when a sync folder
    // is passed in.
    MegaIgnoreManager makeManager() const
    {
        MegaIgnoreManager manager;
        manager.setDefaultIgnorePath(defaultFilePath());
        manager.setOutputIgnorePath(outputFilePath());
        return manager;
    }

private:
    QTemporaryDir mDataDir;
    QTemporaryDir mSyncDir;
};

TEST_CASE_METHOD(MegaIgnoreManagerTests, "restoreDefaults() creates the file from the default one")
{
    writeDefaultFile();
    REQUIRE_FALSE(QFileInfo::exists(outputFilePath()));

    auto manager(makeManager());
    const auto result = manager.restoreDefaults();

    CHECK(result == MegaIgnoreManager::RestoreDefaultsResult::Restored);
    REQUIRE(QFileInfo::exists(outputFilePath()));
    CHECK(readFile(outputFilePath()) == QString::fromUtf8(DEFAULT_RULES));
}

TEST_CASE_METHOD(MegaIgnoreManagerTests, "restoreDefaults() replaces the existing rules")
{
    writeDefaultFile();
    writeFile(outputFilePath(), QString::fromUtf8("-:my_own_rule\n"));

    auto manager(makeManager());
    manager.restoreDefaults();

    CHECK(readFile(outputFilePath()) == QString::fromUtf8(DEFAULT_RULES));
}

TEST_CASE_METHOD(MegaIgnoreManagerTests, "restoreDefaults() keeps the file hidden")
{
    // The default file is deliberately left visible, so a hidden result can only come from
    // restoreDefaults() itself and not from an attribute inherited through the copy
    writeDefaultFile();

    auto manager(makeManager());
    manager.restoreDefaults();

    REQUIRE(QFileInfo::exists(outputFilePath()));
#ifdef Q_OS_WINDOWS
    CHECK(QFileInfo(outputFilePath()).isHidden());
#endif
}

TEST_CASE_METHOD(MegaIgnoreManagerTests, "restoreDefaults() keeps an already hidden file hidden")
{
    writeDefaultFile();
    writeFile(outputFilePath(), QString::fromUtf8("-:my_own_rule\n"));
    REQUIRE(Platform::getInstance()->setHidden(outputFilePath()));

    auto manager(makeManager());
    manager.restoreDefaults();

    // The file is replaced, so the attribute has to be reapplied.
    REQUIRE(QFileInfo::exists(outputFilePath()));
    CHECK(readFile(outputFilePath()) == QString::fromUtf8(DEFAULT_RULES));
#ifdef Q_OS_WINDOWS
    CHECK(QFileInfo(outputFilePath()).isHidden());
#endif
}

TEST_CASE_METHOD(MegaIgnoreManagerTests,
                 "restoreDefaults() reports RestoredButNotHidden when hiding fails")
{
    writeDefaultFile();

    auto manager(makeManager());
    manager.setHiddenApplier(
        [](const QString&)
        {
            return false;
        });
    const auto result = manager.restoreDefaults();

    CHECK(result == MegaIgnoreManager::RestoreDefaultsResult::RestoredButNotHidden);
    // The exclusions themselves must still be restored correctly - only hiding failed.
    CHECK(readFile(outputFilePath()) == QString::fromUtf8(DEFAULT_RULES));
}

TEST_CASE_METHOD(MegaIgnoreManagerTests,
                 "setHiddenApplier() with an empty function falls back to the real implementation")
{
    writeDefaultFile();

    auto manager(makeManager());
    manager.setHiddenApplier(nullptr);

    // restoreDefaults() calls the applier unconditionally; an empty std::function left in
    // place would throw std::bad_function_call instead of restoring anything.
    const auto result = manager.restoreDefaults();
    CHECK(result == MegaIgnoreManager::RestoreDefaultsResult::Restored);
}

TEST_CASE_METHOD(MegaIgnoreManagerTests,
                 "restoreDefaults() removes .megaignore when there is no default file to copy from")
{
    REQUIRE_FALSE(QFileInfo::exists(defaultFilePath()));
    writeFile(outputFilePath(), QString::fromUtf8("-:my_own_rule\n"));

    auto manager(makeManager());
    const auto result = manager.restoreDefaults();

    // Deleting it is only half the story: it's the caller's job to force a sync
    // restart, since the SDK only regenerates the file (hidden, from its own
    // compiled-in defaults) on a transition into RUNSTATE_RUNNING.
    CHECK(result == MegaIgnoreManager::RestoreDefaultsResult::RemovedPendingSdkRegeneration);
    CHECK_FALSE(QFileInfo::exists(outputFilePath()));
}

TEST_CASE_METHOD(MegaIgnoreManagerTests,
                 "restoreDefaults() reports success when .megaignore is already absent and there "
                 "is no default file")
{
    REQUIRE_FALSE(QFileInfo::exists(defaultFilePath()));
    REQUIRE_FALSE(QFileInfo::exists(outputFilePath()));

    auto manager(makeManager());
    const auto result = manager.restoreDefaults();

    // There's nothing to remove, but that's already the desired end state, not a failure.
    CHECK(result == MegaIgnoreManager::RestoreDefaultsResult::RemovedPendingSdkRegeneration);
}

#ifdef Q_OS_WINDOWS
TEST_CASE_METHOD(MegaIgnoreManagerTests,
                 "restoreDefaults() preserves existing rules when replacement fails")
{
    writeDefaultFile();
    const QString originalRules(QString::fromUtf8("-:my_own_rule\n"));
    writeFile(outputFilePath(), originalRules);

    QFile outputFile(outputFilePath());
    REQUIRE(outputFile.setPermissions(QFileDevice::ReadOwner));

    auto manager(makeManager());
    const auto result = manager.restoreDefaults();

    const QString rulesAfterRestore(readFile(outputFilePath()));
    outputFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    CHECK(result == MegaIgnoreManager::RestoreDefaultsResult::Failed);
    CHECK(rulesAfterRestore == originalRules);
}
#endif

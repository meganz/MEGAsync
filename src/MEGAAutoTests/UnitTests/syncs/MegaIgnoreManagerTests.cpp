#include "MegaIgnoreManager.h"
#include <catch.hpp>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include <memory>
#include <string>

/*
 * What reaches the file is the property that matters: the rules are joined with "\n", so a pattern
 * that carries a line terminator would not add one rule but as many as it carries. These tests go
 * through MegaIgnoreManager and then read the .megaignore back.
 *
 * The manager is constructed on a directory with no .megaignore in it, because parsing an existing
 * one asks the MegaApi for its CRC and the test binary has no MegaApi.
 */

namespace
{
QString ignoreFilePath(const QTemporaryDir& dir)
{
    return dir.path() + QDir::separator() +
           QString::fromUtf8(MegaIgnoreManager::MEGA_IGNORE_FILE_NAME);
}

QString readIgnoreFile(const QTemporaryDir& dir)
{
    QFile file(ignoreFilePath(dir));
    if (!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    return in.readAll();
}

// The rule text depends on the platform (the strategy character follows the filesystem's case
// sensitivity), so the assertions below are about the shape of the file rather than its exact
// bytes: how many lines it has, and what is not in them.
int lineCount(const QString& contents)
{
    return contents.isEmpty() ? 0 : static_cast<int>(contents.split(QLatin1Char('\n')).size());
}
}

TEST_CASE("MegaIgnoreManager refuses a name rule whose pattern would span lines")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    MegaIgnoreManager manager(dir.path(), false);

    SECTION("the rule is not created and nothing is written")
    {
        CHECK(manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                  QString::fromUtf8("holiday.jpg\n-:*"),
                                  MegaIgnoreNameRule::Target::f) == nullptr);

        CHECK(manager.applyChanges() == MegaIgnoreManager::ApplyChangesError::NO_UPDATE_NEEDED);
        CHECK_FALSE(QFile::exists(ignoreFilePath(dir)));
    }

    SECTION("a legitimate rule alongside it is still written, on its own single line")
    {
        CHECK(manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                  QString::fromUtf8("holiday.jpg\n-:*"),
                                  MegaIgnoreNameRule::Target::f) == nullptr);
        REQUIRE(manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                    QString::fromUtf8("beach.jpg"),
                                    MegaIgnoreNameRule::Target::f) != nullptr);

        REQUIRE(manager.applyChanges() == MegaIgnoreManager::ApplyChangesError::OK);

        const auto contents(readIgnoreFile(dir));
        CAPTURE(contents.toStdString());
        CHECK(lineCount(contents) == 1);
        CHECK(contents.contains(QString::fromUtf8("beach.jpg")));
        // Not the "-:*" payload itself: that is a substring of any legitimate extension rule. The
        // refused rule's own name is what may not appear.
        CHECK_FALSE(contents.contains(QString::fromUtf8("holiday")));
    }
}

TEST_CASE("MegaIgnoreManager refuses an extension rule whose pattern would span lines")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    MegaIgnoreManager manager(dir.path(), false);

    CHECK(manager.addExtensionRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                   QString::fromUtf8("jpg\n-:*")) == nullptr);
    REQUIRE(manager.addExtensionRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                     QString::fromUtf8("png")) != nullptr);

    REQUIRE(manager.applyChanges() == MegaIgnoreManager::ApplyChangesError::OK);

    const auto contents(readIgnoreFile(dir));
    CAPTURE(contents.toStdString());
    CHECK(lineCount(contents) == 1);
    CHECK(contents.contains(QString::fromUtf8("png")));
    CHECK_FALSE(contents.contains(QString::fromUtf8("jpg")));
}

TEST_CASE("MegaIgnoreManager drops a rule that reached it without going through addNameRule")
{
    // addRule() takes a rule as it is - this is how copyCurrentRulesTo() moves rules between sync
    // folders - so the writer cannot rely on its callers having checked the pattern.
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    MegaIgnoreManager manager(dir.path(), false);

    auto unwritable = std::make_shared<MegaIgnoreNameRule>(QString::fromUtf8("holiday.jpg\n-:*"),
                                                           MegaIgnoreNameRule::Class::EXCLUDE,
                                                           MegaIgnoreNameRule::Target::f);
    REQUIRE(manager.addRule(unwritable));
    REQUIRE(manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                QString::fromUtf8("beach.jpg"),
                                MegaIgnoreNameRule::Target::f) != nullptr);

    // The write happens - the legitimate rule is not held hostage - but it is reported as a
    // failure, because one of the rules asked for is not in the file.
    CHECK(manager.applyChanges() == MegaIgnoreManager::ApplyChangesError::UNSUPPORTED_PATTERN);

    const auto contents(readIgnoreFile(dir));
    CAPTURE(contents.toStdString());
    CHECK(lineCount(contents) == 1);
    CHECK(contents.contains(QString::fromUtf8("beach.jpg")));
    CHECK_FALSE(contents.contains(QString::fromUtf8("holiday")));
    CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(contents));
}

TEST_CASE("MegaIgnoreManager reports NO_WRITE_PERMISSION when the file cannot be written")
{
    // Constructed from an empty directory, so parsing at construction time does not ask
    // MegaApi for a CRC (see the file comment above); the write target is redirected below
    // to the locked file actually under test.
    QTemporaryDir syncDir;
    REQUIRE(syncDir.isValid());
    MegaIgnoreManager manager(syncDir.path(), false);

    QTemporaryDir outputDir;
    REQUIRE(outputDir.isValid());
    const QString originalRules(QString::fromUtf8("-:my_own_rule\n"));
    {
        QFile file(ignoreFilePath(outputDir));
        REQUIRE(file.open(QIODevice::WriteOnly));
        file.write(originalRules.toUtf8());
        file.close();
    }
    QFile outputFile(ignoreFilePath(outputDir));
    REQUIRE(outputFile.setPermissions(QFileDevice::ReadOwner));

    manager.setOutputIgnorePath(ignoreFilePath(outputDir));
    REQUIRE(manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                QString::fromUtf8("beach.jpg"),
                                MegaIgnoreNameRule::Target::f) != nullptr);

    const auto result = manager.applyChanges();
    const auto contentsAfter(readIgnoreFile(outputDir));

    // Restore write access before the temporary directory tries to clean itself up.
    outputFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    CHECK(result == MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION);
    // The rule that could not be written did not overwrite what was already on disk.
    CHECK(contentsAfter == originalRules);
}

TEST_CASE("MegaIgnoreManager::ApplyChangesError orders every failure after the success values")
{
    // Callers test `result < NO_WRITE_PERMISSION` for success, so a new failure added before it
    // would be read as one.
    CHECK(MegaIgnoreManager::ApplyChangesError::OK <
          MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION);
    CHECK(MegaIgnoreManager::ApplyChangesError::NO_UPDATE_NEEDED <
          MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION);
    CHECK_FALSE(MegaIgnoreManager::ApplyChangesError::UNSUPPORTED_PATTERN <
                MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION);
}

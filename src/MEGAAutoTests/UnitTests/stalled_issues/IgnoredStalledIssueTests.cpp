#include "IgnoredStalledIssue.h"
#include <catch.hpp>

#include <QDir>

#include <string>

/*
 * The rule written for an ignored path is relative to the sync root, so the sync root is not part
 * of it. Deciding whether an item can be ignored has to be a question about the same string the
 * writer receives, or a control character somewhere above the sync root refuses items whose own
 * names are perfectly fine.
 */

namespace
{
class FakeSync: public mega::MegaSync
{
public:
    FakeSync(const QString& localFolder, const QString& lastKnownMegaFolder):
        mLocalFolder(localFolder.toUtf8()),
        mLastKnownMegaFolder(lastKnownMegaFolder.toUtf8())
    {}

    const char* getLocalFolder() const override
    {
        return mLocalFolder.constData();
    }

    const char* getLastKnownMegaFolder() const override
    {
        return mLastKnownMegaFolder.constData();
    }

private:
    QByteArray mLocalFolder;
    QByteArray mLastKnownMegaFolder;
};

IgnoredStalledIssue::IgnoredPath localPath(const QString& path)
{
    return IgnoredStalledIssue::IgnoredPath(path,
                                            IgnoredStalledIssue::IgnoredPath::IgnorePathSide::LOCAL,
                                            MegaIgnoreNameRule::Target::f);
}

IgnoredStalledIssue::IgnoredPath remotePath(const QString& path)
{
    return IgnoredStalledIssue::IgnoredPath(
        path,
        IgnoredStalledIssue::IgnoredPath::IgnorePathSide::REMOTE,
        MegaIgnoreNameRule::Target::f);
}

// Built from the temporary directory so the assertions hold on every platform. Nothing here has to
// exist on disk: the pattern is computed from the strings.
const QString localRoot = QDir::tempPath() + QLatin1String("/megasync-tests-sync");
const QString megaRoot = QLatin1String("/Root/Shared holiday");
}

TEST_CASE("IgnoredStalledIssue::ignoreRulePattern() is relative to the side's root")
{
    const FakeSync sync(localRoot, megaRoot);

    SECTION("a local path drops the sync's local folder")
    {
        const auto result(IgnoredStalledIssue::ignoreRulePattern(
            localPath(localRoot + QLatin1String("/holiday.jpg")),
            sync));
        CHECK(result.toStdString() == std::string("holiday.jpg"));
    }

    SECTION("a cloud path drops the sync's last known MEGA folder")
    {
        const auto result(IgnoredStalledIssue::ignoreRulePattern(
            remotePath(megaRoot + QLatin1String("/holiday.jpg")),
            sync));
        CHECK(result.toStdString() == std::string("holiday.jpg"));
    }

    SECTION("the folders below the root are kept")
    {
        const auto result(IgnoredStalledIssue::ignoreRulePattern(
            localPath(localRoot + QLatin1String("/photos/2026/holiday.jpg")),
            sync));
        CHECK(result.toStdString() == std::string("photos/2026/holiday.jpg"));
    }
}

TEST_CASE("Only the part of the path that gets written decides whether it can be ignored")
{
    SECTION("a control character in the sync root is not the item's problem")
    {
        // The user's own folder, on a filesystem that allows the character. The item below it is
        // ignorable, and judging the absolute path would have refused it.
        const QString awkwardRoot(QDir::tempPath() + QLatin1String("/megasync\ttests"));
        const FakeSync sync(awkwardRoot, megaRoot);

        const auto result(IgnoredStalledIssue::ignoreRulePattern(
            localPath(awkwardRoot + QLatin1String("/holiday.jpg")),
            sync));

        CHECK(result.toStdString() == std::string("holiday.jpg"));
        CHECK(MegaIgnoreRule::isPatternWritable(result));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(awkwardRoot + QLatin1String("/holiday.jpg")));
    }

    SECTION("a control character in a cloud folder above the sync root is not either")
    {
        const QString awkwardMegaRoot(QLatin1String("/Root/Shared\nholiday"));
        const FakeSync sync(localRoot, awkwardMegaRoot);

        const auto result(IgnoredStalledIssue::ignoreRulePattern(
            remotePath(awkwardMegaRoot + QLatin1String("/holiday.jpg")),
            sync));

        CHECK(result.toStdString() == std::string("holiday.jpg"));
        CHECK(MegaIgnoreRule::isPatternWritable(result));
    }

    SECTION("a control character in the item's own name still refuses it")
    {
        const FakeSync sync(localRoot, megaRoot);

        const auto result(IgnoredStalledIssue::ignoreRulePattern(
            localPath(localRoot + QLatin1String("/holiday.jpg\n-:*")),
            sync));

        CHECK(result.toStdString() == std::string("holiday.jpg\n-:*"));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(result));
    }
}

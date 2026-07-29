#include "MegaIgnoreRules.h"
#include <catch.hpp>

#include <string>

/*
 * A .megaignore rule occupies exactly one line and the format has no escape syntax, so a pattern
 * carrying a control character either splits the rule in two or has no way of being written at
 * all. These tests pin the two checks that keep an attacker-controlled node name from becoming
 * extra rules in the victim's file.
 */

namespace
{
QString pattern(const char* text)
{
    return QString::fromUtf8(text);
}
}

TEST_CASE("MegaIgnoreRule::isPatternWritable() refuses every control character")
{
    SECTION("patterns a user can legitimately ask for are accepted")
    {
        CHECK(MegaIgnoreRule::isPatternWritable(pattern("holiday.jpg")));
        CHECK(MegaIgnoreRule::isPatternWritable(pattern("*.jpg")));
        CHECK(MegaIgnoreRule::isPatternWritable(pattern("photos/summer 2026/holiday.jpg")));
        CHECK(MegaIgnoreRule::isPatternWritable(pattern("ñ 日本語 emoji 🙂")));
        // Nothing to refuse in an empty pattern; whether it is worth writing is not this check.
        CHECK(MegaIgnoreRule::isPatternWritable(QString()));
    }

    SECTION("a line terminator would turn one rule into several")
    {
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday.jpg\n-:*")));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday.jpg\r-:*")));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday.jpg\r\n+:*")));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("\nholiday.jpg")));
    }

    SECTION("control characters that cannot split a rule are refused too")
    {
        // They have no legitimate use in a pattern, and they are a terminal-escape primitive on
        // the way to the log.
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday\tjpg")));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday\x1b[2Kjpg")));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday\x7fjpg")));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(pattern("holiday\vjpg")));
        // C1 range, of which NEL (U+0085) is the one some readers treat as a line break.
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(QString(QChar(0x0085))));
        CHECK_FALSE(MegaIgnoreRule::isPatternWritable(QString(QChar(0x009b))));
    }

    SECTION("the Unicode line separators are not refused")
    {
        // Documenting a known limit rather than asking for one: the SDK reads the file as bytes
        // and splits on CR and LF, so U+2028/U+2029 cannot split a rule there. They are category
        // Zl/Zp, not Cc, so this check lets them through.
        CHECK(MegaIgnoreRule::isPatternWritable(QString(QChar(0x2028))));
        CHECK(MegaIgnoreRule::isPatternWritable(QString(QChar(0x2029))));
    }
}

TEST_CASE("MegaIgnoreRule::spansMultipleLines() covers only what breaks the file")
{
    SECTION("a line terminator is what makes one rule become several")
    {
        CHECK(MegaIgnoreRule::spansMultipleLines(pattern("-:holiday.jpg\n-:*")));
        CHECK(MegaIgnoreRule::spansMultipleLines(pattern("-:holiday.jpg\r-:*")));
        CHECK(MegaIgnoreRule::spansMultipleLines(pattern("-:holiday.jpg\r\n-:*")));
    }

    SECTION("anything else stays on its line")
    {
        CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(pattern("-:holiday.jpg")));
        CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(QString()));
        // The distinction this check exists for: a line already in the user's .megaignore that
        // carries one of these is written back rather than dropped.
        CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(pattern("-:holiday\tjpg")));
        CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(pattern("# a\vcomment")));
        CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(QString(QChar(0x2028))));
    }
}

TEST_CASE("MegaIgnoreNameRule keeps the pattern it had when a new one is refused")
{
    MegaIgnoreNameRule rule(pattern("holiday.jpg"),
                            MegaIgnoreNameRule::Class::EXCLUDE,
                            MegaIgnoreNameRule::Target::f);

    REQUIRE(rule.getModifiedRule().toStdString() == std::string("-f:holiday.jpg"));

    SECTION("a writable pattern replaces it")
    {
        CHECK(rule.setPattern(pattern("beach.jpg")));
        CHECK(rule.getModifiedRule().toStdString() == std::string("-f:beach.jpg"));
    }

    SECTION("an injected rule is refused and changes nothing")
    {
        CHECK_FALSE(rule.setPattern(pattern("beach.jpg\n-:*")));
        CHECK(rule.getModifiedRule().toStdString() == std::string("-f:holiday.jpg"));
        CHECK_FALSE(MegaIgnoreRule::spansMultipleLines(rule.getModifiedRule()));
    }

    SECTION("setting the pattern it already has is not a refusal")
    {
        CHECK(rule.setPattern(pattern("holiday.jpg")));
        CHECK(rule.getModifiedRule().toStdString() == std::string("-f:holiday.jpg"));
    }
}

TEST_CASE("MegaIgnoreExtensionRule keeps the extension it had when a new one is refused")
{
    MegaIgnoreExtensionRule rule(MegaIgnoreNameRule::Class::EXCLUDE, pattern("jpg"));

    REQUIRE(rule.getModifiedRule().toStdString() == std::string("-:*.jpg"));
    REQUIRE(rule.extension().toStdString() == std::string("jpg"));

    SECTION("a writable extension replaces it")
    {
        CHECK(rule.setPattern(pattern("png")));
        CHECK(rule.extension().toStdString() == std::string("png"));
        CHECK(rule.getModifiedRule().toStdString() == std::string("-:*.png"));
    }

    SECTION("an injected rule is refused and changes nothing")
    {
        CHECK_FALSE(rule.setPattern(pattern("png\n-:*")));
        CHECK(rule.extension().toStdString() == std::string("jpg"));
        CHECK(rule.getModifiedRule().toStdString() == std::string("-:*.jpg"));
    }
}

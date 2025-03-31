#ifndef MEGACATCHREPORTERUTILITIES_H
#define MEGACATCHREPORTERUTILITIES_H

#define CATCH_CONFIG_EXTERNAL_INTERFACES
#include <catch/catch.hpp>

#include <map>

struct AssertionFailureData
{
    AssertionFailureData(const Catch::AssertionStats& _stats,
                         const std::vector<Catch::SectionInfo>& _sectionStack,
                         const std::string& _testCaseName,
                         const std::string& _expandedExpression):
        mStats(_stats),
        mSectionStack(_sectionStack),
        mTestCaseName(_testCaseName),
        mExpandedExpression(_expandedExpression)
    {}

    Catch::AssertionStats mStats;
    std::vector<Catch::SectionInfo> mSectionStack;
    std::string mTestCaseName;
    std::string mExpandedExpression;
};

struct FailureTypeData
{
    FailureTypeData() = default;

    FailureTypeData(const std::string& _title,
                    const Catch::Colour::Code& _colour,
                    const std::string& _additionalInfo = ""):
        mTitle(_title),
        mColour(_colour),
        mAdditionalInfo(_additionalInfo)
    {}

    std::string mTitle;
    Catch::Colour::Code mColour;
    std::string mAdditionalInfo;
};

class ResultHandler
{
public:
    ResultHandler();

    FailureTypeData getData(Catch::ResultWas::OfType type);

private:
    void populateResultMap();

    std::map<Catch::ResultWas::OfType, FailureTypeData> mFailTypeData;
};

#endif // MEGACATCHREPORTERUTILITIES_H

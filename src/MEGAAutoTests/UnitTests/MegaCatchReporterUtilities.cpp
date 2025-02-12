#include "MegaCatchReporterUtilities.h"

ResultHandler::ResultHandler()
{
    populateResultMap();
}

FailureTypeData ResultHandler::getData(Catch::ResultWas::OfType type)
{
    auto itFailureType = mFailTypeData.find(type);
    if (itFailureType != mFailTypeData.end())
    {
        return itFailureType->second;
    }
    return FailureTypeData("** Internal Error **", Catch::Colour::Error);
}

void ResultHandler::populateResultMap()
{
    mFailTypeData[Catch::ResultWas::Ok] = FailureTypeData("Passed", Catch::Colour::Success);
    mFailTypeData[Catch::ResultWas::ExpressionFailed] =
        FailureTypeData("Failed", Catch::Colour::Error);
    mFailTypeData[Catch::ResultWas::ThrewException] =
        FailureTypeData("Received Exception", Catch::Colour::Error);
    mFailTypeData[Catch::ResultWas::FatalErrorCondition] =
        FailureTypeData("Crashed", Catch::Colour::Error);
    mFailTypeData[Catch::ResultWas::DidntThrowException] =
        FailureTypeData("Failed",
                        Catch::Colour::Error,
                        "No exception was thrown where one was expected");
    mFailTypeData[Catch::ResultWas::Info] = FailureTypeData("Info", Catch::Colour::None);
    mFailTypeData[Catch::ResultWas::Warning] = FailureTypeData("Warning", Catch::Colour::Yellow);
    mFailTypeData[Catch::ResultWas::ExplicitFailure] =
        FailureTypeData("Failed", Catch::Colour::Error, "Failed explicitly with ");
}

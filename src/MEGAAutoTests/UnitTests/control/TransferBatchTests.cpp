#include "TransferBatch.h"
#include <catch.hpp>
#include <trompeloeil.hpp>

#include <QDir>

TEST_CASE("class TransferBatchTests isEmpty()")
{
    SUCCEED("Already tested with onScanCompleted()");
}

TEST_CASE("class TransferBatchTests cancel()")
{
    SUCCEED("Not testable : it creates a cancel token internally."
            "To test it would need mocking the cancel token, but as it is "
            "internal, the design of the class would need to be altered.");
}

TEST_CASE("class TransferBatchTests onScanCompleted()")
{
    const unsigned long long appDataId = 1234;
    TransferBatch batch(appDataId);
    CHECK(batch.isEmpty() == false);

    batch.onScanCompleted(456);
    CHECK(batch.isEmpty() == false);

    batch.onScanCompleted(appDataId);
    CHECK(batch.isEmpty() == true);
}

TEST_CASE("class TransferBatchTests description()")
{
    SUCCEED("No use to test : this method is meant only for debug purposes");
}

TEST_CASE("class TransferBatchTests getCancelTokenPtr()")
{
    SUCCEED("No use to test : always returns is internal cancelPointer");
}

TEST_CASE("class TransferBatchTests getCancelToken()")
{
    SUCCEED("No use to test : always returns is internal cancelPointer");
}

TEST_CASE("class BlockingBatchTests add()")
{
    SECTION("Adding a batch makes object valid")
    {
        BlockingBatch batch;
        CHECK(batch.isValid() == false);

        batch.add(std::make_shared<TransferBatch>(123));
        CHECK(batch.isValid() == true);
    }
}

TEST_CASE("class BlockingBatchTests removeBatch()")
{
    SECTION("Removing without batch does not crash")
    {
        BlockingBatch batch;
        batch.removeBatch();
        CHECK(batch.isValid() == false);
    }

    SECTION("Removing existing batch invalidates object")
    {
        BlockingBatch batch;
        batch.add(std::make_shared<TransferBatch>(123));
        batch.removeBatch();
        CHECK(batch.isValid() == false);
    }
}

class TransferBatchMock: public TransferBatch
{
public:
    TransferBatchMock():
        TransferBatch(123)
    {}

    MAKE_MOCK0(cancel, void());
    MAKE_MOCK1(onScanCompleted, void(unsigned long long));
};

TEST_CASE("class BlockingBatchTests cancelTransfer()")
{
    SECTION("Cancelling invalid object does nothing")
    {
        BlockingBatch batch;

        batch.cancelTransfer();

        CHECK(batch.isCancelled() == false);
        CHECK(batch.isValid() == false);
    }

    SECTION("Cancelling valid object marks it as cancelled and calls cancel() on batch")
    {
        BlockingBatch batch;
        auto transferMock = std::make_shared<TransferBatchMock>();
        batch.add(transferMock);

        REQUIRE_CALL(*transferMock, cancel()).TIMES(1);
        REQUIRE_CALL(*transferMock, onScanCompleted(123ULL)).TIMES(0);

        batch.cancelTransfer();

        CHECK(batch.isCancelled() == true);
        CHECK(batch.isValid() == true);
    }
}

TEST_CASE("class BlockingBatchTests onScanCompleted()")
{
    SUCCEED("No use to test : redundant with TransferBatch::onScanCompleted()."
            "A test would be useful only with a mocking library.");
}

TEST_CASE("class BlockingBatchTests isBlockingStageFinished()")
{
    BlockingBatch batch;
    const unsigned long long appDataId = 123;
    auto transferBatch = std::make_shared<TransferBatch>(appDataId);

    SECTION("True if there is no batch")
    {
        CHECK(batch.isBlockingStageFinished() == true);
    }

    SECTION("False if there is a batch and it is not completed")
    {
        batch.add(transferBatch);
        CHECK(batch.isBlockingStageFinished() == false);
    }

    SECTION("True if there is a batch and it is completed")
    {
        batch.add(transferBatch);
        transferBatch->onScanCompleted(appDataId);
        CHECK(batch.isBlockingStageFinished() == true);
    }
}

TEST_CASE("class BlockingBatchTests setAsUnblocked()")
{
    SECTION("Clears batch and resets isCancelled flag")
    {
        BlockingBatch batch;
        auto transferBatch = std::make_shared<TransferBatch>(123);
        batch.add(transferBatch);
        CHECK(batch.isValid() == true);
        CHECK(batch.isCancelled() == false);

        batch.cancelTransfer();
        CHECK(batch.isValid() == true);
        CHECK(batch.isCancelled() == true);

        batch.setAsUnblocked();

        CHECK(batch.isValid() == false);
        CHECK(batch.isCancelled() == false);
    }
}

TEST_CASE("class BlockingBatchTests hasCancelToken()")
{
    BlockingBatch batch;
    SECTION("False if there is no valid batch")
    {
        CHECK(batch.hasCancelToken() == false);
    }

    SECTION("True if there is a valid batch")
    {
        batch.add(std::make_shared<TransferBatch>(123));
        CHECK(batch.hasCancelToken() == true);
    }
}

TEST_CASE("class BlockingBatchTests isValid()")
{
    SUCCEED("Already tested with add() and removeBatch()");
}

TEST_CASE("class BlockingBatchTests isCancelled()")
{
    SUCCEED("Already tested with cancelTransfer()");
}

TEST_CASE("class BlockingBatchTests hasNodes()")
{
    BlockingBatch batch;
    const unsigned long long appDataId = 123;
    auto transferBatch = std::make_shared<TransferBatch>(appDataId);
    SECTION("False when there is no batch")
    {
        CHECK(batch.hasNodes() == false);
    }

    SECTION("True when there is a batch and it has nodes")
    {
        batch.add(transferBatch);
        CHECK(batch.hasNodes() == true);
    }

    SECTION("False when there is an empty batch")
    {
        batch.add(transferBatch);
        transferBatch->onScanCompleted(appDataId);
        CHECK(batch.hasNodes() == false);
    }
}

TEST_CASE("class BlockingBatchTests getCancelToken()")
{
    SUCCEED("Not testable : tokens are generated internally."
            "Even in case of not having a valid batch a new one is generated.");
}

TEST_CASE("class BlockingBatchTests description()")
{
    SUCCEED("No use for a test. This is used for debug purposes.");
}

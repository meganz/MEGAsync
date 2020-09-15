#include "MegaApplication.h"
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <trompeloeil_catch.hpp>

int main( int argc, char* argv[] )
{
    MegaApplication app(argc, argv);

    int result = Catch::Session().run( argc, argv );
    return ( result < 0xff ? result : 0xff );
}

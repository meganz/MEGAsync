#ifndef TESTSINGLETONS_H
#define TESTSINGLETONS_H

#include "Platform.h"

// The unit test runner constructs MegaApplication but never calls its initialize(), so the
// process wide singletons the app normally sets up are missing. Tests that need one have to
// create it themselves.

// Platform::getInstance() asserts on a null instance. The instance is process wide, so create
// it only once for the whole test binary.
inline void ensurePlatformCreated()
{
    static bool created = false;
    if (!created)
    {
        Platform::create();
        created = true;
    }
}

#endif // TESTSINGLETONS_H

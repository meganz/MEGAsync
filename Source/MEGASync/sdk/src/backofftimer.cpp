/**
 * @file backofftimer.cpp
 * @brief Generic timer facility with exponential backoff
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "mega/waiter.h"
#include "mega/backofftimer.h"

namespace mega {
// timer with capped exponential backoff
BackoffTimer::BackoffTimer()
{
    reset();
}

void BackoffTimer::reset()
{
    next = 0;
    delta = 1;
}

void BackoffTimer::backoff()
{
    next = Waiter::ds + delta;

    delta <<= 1;

    if (delta > 36000)
    {
        delta = 36000;
    }
}

void BackoffTimer::backoff(dstime newdelta)
{
    next = Waiter::ds + newdelta;
    delta = newdelta;
}

bool BackoffTimer::armed() const
{
    return !next || Waiter::ds >= next;
}

bool BackoffTimer::arm()
{
    if (next + delta > Waiter::ds)
    {
        next = Waiter::ds;
        delta = 1;

        return true;
    }

    return false;
}

dstime BackoffTimer::retryin()
{
    if (armed())
    {
        return 0;
    }

    return next - Waiter::ds;
}

dstime BackoffTimer::backoffdelta()
{
    return delta;
}

dstime BackoffTimer::nextset() const
{
    return (int)next;
}

// event in the future: potentially updates waituntil
// event in the past: zeros out waituntil and clears event
void BackoffTimer::update(dstime* waituntil)
{
    if (next)
    {
        if (next <= Waiter::ds)
        {
            *waituntil = 0;
            next = 1;
        }
        else if (next < *waituntil)
        {
            *waituntil = next;
        }
    }
}
} // namespace

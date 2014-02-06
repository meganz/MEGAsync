/**
 * @file json.cpp
 * @brief Linear non-strict JSON scanner
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

#include "mega/json.h"
#include "mega/base64.h"
#include "mega/megaclient.h"

namespace mega {
// store array or object in string s
// reposition after object
bool JSON::storeobject(string* s)
{
    int openobject[2] = { 0 };
    const char* ptr;

    while (*pos > 0 && *pos <= ' ')
    {
        pos++;
    }

    if (*pos == ',')
    {
        pos++;
    }

    ptr = pos;

    for (; ; )
    {
        if (( *ptr == '[' ) || ( *ptr == '{' ))
        {
            openobject[*ptr == '[']++;
        }
        else if (( *ptr == ']' ) || ( *ptr == '}' ))
        {
            openobject[*ptr == ']']--;
        }
        else if (*ptr == '"')
        {
            ptr++;
            while (*ptr != '"' || ptr[-1] == '\\')
            {
                ptr++;
            }
        }
        else if ((( *ptr >= '0' ) && ( *ptr <= '9' )) || ( *ptr == '-' ) || ( *ptr == '.' ))
        {
            ptr++;
            while (( *ptr >= '0' && *ptr <= '9' ) || *ptr == '.' || *ptr == 'e' || *ptr == 'E')
            {
                ptr++;
            }

            ptr--;
        }
        else if (( *ptr != ':' ) && ( *ptr != ',' ))
        {
            return false;
        }

        ptr++;

        if (!openobject[0] && !openobject[1])
        {
            if (s)
            {
                if (*pos == '"')
                {
                    s->assign(pos + 1, ptr - pos - 2);
                }
                else
                {
                    s->assign(pos, ptr - pos);
                }
            }
            pos = ptr;
            return true;
        }
    }
}

bool JSON::isnumeric()
{
    if (*pos == ',')
    {
        pos++;
    }

    const char* ptr = pos;

    if (*ptr == '-')
    {
        ptr++;
    }

    return *ptr >= '0' && *ptr <= '9';
}

nameid JSON::getnameid(const char* ptr)
{
    nameid id = EOO;

    while (*ptr && *ptr != '"')
    {
        id = ( id << 8 ) + *ptr++;
    }

    return id;
}

// pos points to [,]"name":...
// returns nameid and repositons pos after :
// no unescaping supported
nameid JSON::getnameid()
{
    const char* ptr = pos;
    nameid id = 0;

    if (( *ptr == ',' ) || ( *ptr == ':' ))
    {
        ptr++;
    }

    if (*ptr++ == '"')
    {
        while (*ptr && *ptr != '"')
        {
            id = ( id << 8 ) + *ptr++;
        }

        pos = ptr + 2;
    }

    return id;
}

// specific string comparison/skipping
bool JSON::is(const char* value)
{
    if (*pos == ',')
    {
        pos++;
    }

    if (*pos != '"')
    {
        return false;
    }

    int t = strlen(value);

    if (memcmp(pos + 1, value, t) || ( pos[t + 1] != '"' ))
    {
        return false;
    }

    pos += t + 2;

    return true;
}

// base64-decode binary value to designated fixed-length buffer
int JSON::storebinary(byte* dst, int dstlen)
{
    int l = 0;

    if (*pos == ',')
    {
        pos++;
    }

    if (*pos == '"')
    {
        l = Base64::atob(pos + 1, dst, dstlen);

        // skip string
            storeobject();
    }

    return l;
}

// base64-decode binary value to designated string
bool JSON::storebinary(string* dst)
{
    if (*pos == ',')
    {
        pos++;
    }

    if (*pos == '"')
    {
        const char* ptr;

        if (!( ptr = strchr(pos + 1, '"')))
        {
            return false;
        }

        dst->resize(( ptr - pos - 1 ) / 4 * 3 + 3);
        dst->resize(Base64::atob(pos + 1, (byte*)dst->data(), dst->size()));

        // skip string
            storeobject();
    }

    return true;
}

// decode handle
handle JSON::gethandle(int size)
{
    byte buf[9] = { 0 };

    // no arithmetic or semantic comparisons will be performed on handles, so
    // no endianness issues
    if (storebinary(buf, sizeof buf) == size)
    {
        return *(handle*)buf;
    }

    return UNDEF;
}

// decode integer
m_off_t JSON::getint()
{
    const char* ptr;

    if (( *pos == ':' ) || ( *pos == ',' ))
    {
        pos++;
    }

    ptr = pos;
    if (*ptr == '"')
    {
        ptr++;
    }

    if ((( *ptr < '0' ) || ( *ptr > '9' )) && ( *ptr != '-' ))
    {
        return -1;
    }

    handle r = atoll(ptr);
            storeobject();

    return r;
}

// decode float
double JSON::getfloat()
{
    if (( *pos == ':' ) || ( *pos == ',' ))
    {
        pos++;
    }

    if ((( *pos < '0' ) || ( *pos > '9' )) && ( *pos != '-' ) && ( *pos != '.' ))
    {
        return -1;
    }

    double r = atof(pos);

            storeobject();

    return r;
}

// return pointer to JSON payload data
const char* JSON::getvalue()
{
    const char* r;

    if (( *pos == ':' ) || ( *pos == ',' ))
    {
        pos++;
    }

    if (*pos == '"')
    {
        r = pos + 1;
    }
    else
    {
        r = pos;
    }

            storeobject();

    return r;
}

// try to to enter array
bool JSON::enterarray()
{
    if (*pos == ',')
    {
        pos++;
    }

    if (*pos == '[')
    {
        pos++;
        return true;
    }

    return false;
}

// leave array (must be at end of array)
bool JSON::leavearray()
{
    if (*pos == ']')
    {
        pos++;
        return true;
    }

    return false;
}

// try to enter object
bool JSON::enterobject()
{
    if (*pos == '}')
    {
        pos++;
    }
    if (*pos == ',')
    {
        pos++;
    }

    if (*pos == '{')
    {
        pos++;
        return true;
    }

    return false;
}

// leave object (skip remainder)
bool JSON::leaveobject()
{
    for (; ; )
    {
        if (( *pos == ':' ) || ( *pos == ',' ))
        {
            pos++;
        }
        else if (( *pos == '"' )
                || (( *pos >= '0' )
                        && ( *pos <= '9' ))
                        || ( *pos == '-' )
                        || ( *pos == '[' )
                        || ( *pos == '{' ))
        {
            storeobject();
        }
        else
        {
            break;
        }
    }

    if (*pos == '}')
    {
        pos++;
        return true;
    }

    return false;
}

// unescape JSON string (non-strict)
void JSON::unescape(string* s)
{
    char c;
    int l;

    for (unsigned i = 0; i + 1 < s->size(); i++)
    {
        if (( *s )[i] == '\\')
        {
            switch (( *s )[i + 1])
            {
                case 'n':
                    c = '\n';
                    l = 2;
                    break;

                case 'r':
                    c = '\r';
                    l = 2;
                    break;

                case 'b':
                    c = '\b';
                    l = 2;
                    break;

                case 'f':
                    c = '\f';
                    l = 2;
                    break;

                case 't':
                    c = '\t';
                    l = 2;
                    break;

                case 'u':
                    c = ( MegaClient::hexval(( *s )[i + 4]) << 4 ) | MegaClient::hexval(( *s )[i + 5]);
                    l = 6;
                    break;

                default:
                    c = ( *s )[i + 1];
                    l = 2;
            }

            s->replace(i, l, &c, 1);
        }
    }
}

// position at start of object
void JSON::begin(const char* json)
{
    pos = json;
}
} // namespace

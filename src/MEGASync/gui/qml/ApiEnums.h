#ifndef APIENUMS_H
#define APIENUMS_H

#include <QObject>

namespace ApiEnums
{
    Q_NAMESPACE

    /**
     * @brief Declaration of API error codes.
     */
    enum MegaError
    {
        API_OK = 0,                     ///< Everything OK
        API_EINTERNAL = -1,             ///< Internal error.
        API_EARGS = -2,                 ///< Bad arguments.
        API_EAGAIN = -3,                ///< Request failed, retry with exponential back-off.
        API_ERATELIMIT = -4,            ///< Too many requests, slow down.
        API_EFAILED = -5,               ///< Request failed permanently.
        API_ETOOMANY = -6,              ///< Too many requests for this resource.
        API_ERANGE = -7,                ///< Resource access out of range.
        API_EEXPIRED = -8,              ///< Resource expired.
        API_ENOENT = -9,                ///< Resource does not exist.
        API_ECIRCULAR = -10,            ///< Circular linkage.
        API_EACCESS = -11,              ///< Access denied.
        API_EEXIST = -12,               ///< Resource already exists.
        API_EINCOMPLETE = -13,          ///< Request incomplete.
        API_EKEY = -14,                 ///< Cryptographic error.
        API_ESID = -15,                 ///< Bad session ID.
        API_EBLOCKED = -16,             ///< Resource administratively blocked.
        API_EOVERQUOTA = -17,           ///< Quota exceeded.
        API_ETEMPUNAVAIL = -18,         ///< Resource temporarily not available.
        API_ETOOMANYCONNECTIONS = -19,  ///< Too many connections on this resource.
        API_EWRITE = -20,               ///< File could not be written to (or failed post-write integrity check).
        API_EREAD = -21,                ///< File could not be read from (or changed unexpectedly during reading).
        API_EAPPKEY = -22,              ///< Invalid or missing application key.
        API_ESSL = -23,                 ///< SSL verification failed
        API_EGOINGOVERQUOTA = -24,      ///< Not enough quota
        API_EMFAREQUIRED = -26,         ///< Multi-factor authentication required
        API_EMASTERONLY = -27,          ///< Access denied for sub-users (only for business accounts)
        API_EBUSINESSPASTDUE = -28,     ///< Business account expired
        API_EPAYWALL = -29,             ///< Over Disk Quota Paywall

        PAYMENT_ECARD = -101,
        PAYMENT_EBILLING = -102,
        PAYMENT_EFRAUD = -103,
        PAYMENT_ETOOMANY = -104,
        PAYMENT_EBALANCE = -105,
        PAYMENT_EGENERIC = -106,

        LOCAL_ENOSPC = -1000, ///< Insufficient space.
    };
    Q_ENUM_NS(MegaError)

    enum BlockedAccountError {
        ACCOUNT_NOT_BLOCKED = 0,
        ACCOUNT_BLOCKED_EXCESS_DATA_USAGE = 100,        // (deprecated)
        ACCOUNT_BLOCKED_TOS_COPYRIGHT = 200,            // suspended due to copyright violations
        ACCOUNT_BLOCKED_TOS_NON_COPYRIGHT = 300,        // suspended due to multiple breaches of MEGA ToS
        ACCOUNT_BLOCKED_SUBUSER_DISABLED = 400,         // subuser disabled by business administrator
        ACCOUNT_BLOCKED_SUBUSER_REMOVED = 401,          // subuser removed by business administrator
        ACCOUNT_BLOCKED_VERIFICATION_SMS = 500,         // temporary blocked, require SMS verification
        ACCOUNT_BLOCKED_VERIFICATION_EMAIL = 700       // temporary blocked, require email verification
    };
    Q_ENUM_NS(BlockedAccountError)

};

#endif // APIENUMS_H

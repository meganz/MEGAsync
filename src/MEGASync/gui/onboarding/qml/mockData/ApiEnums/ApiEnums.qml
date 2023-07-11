pragma Singleton
import QtQuick 2.12

QtObject {

    /**
     * @brief Declaration of API error codes.
     */
    enum MegaError {
        API_OK = 0,               ///< Everything OK
        API_EINTERNAL,            ///< Internal error.
        API_EARGS,                ///< Bad arguments.
        API_EAGAIN,               ///< Request failed, retry with exponential back-off.
        API_ERATELIMIT,           ///< Too many requests, slow down.
        API_EFAILED,              ///< Request failed permanently.
        API_ETOOMANY,             ///< Too many requests for this resource.
        API_ERANGE,               ///< Resource access out of range.
        API_EEXPIRED,             ///< Resource expired.
        API_ENOENT,               ///< Resource does not exist.
        API_ECIRCULAR,            ///< Circular linkage.
        API_EACCESS,              ///< Access denied.
        API_EEXIST,               ///< Resource already exists.
        API_EINCOMPLETE,          ///< Request incomplete.
        API_EKEY,                 ///< Cryptographic error.
        API_ESID,                 ///< Bad session ID.
        API_EBLOCKED,             ///< Resource administratively blocked.
        API_EOVERQUOTA,           ///< Quota exceeded.
        API_ETEMPUNAVAIL,         ///< Resource temporarily not available.
        API_ETOOMANYCONNECTIONS,  ///< Too many connections on this resource.
        API_EWRITE,               ///< File could not be written to (or failed post-write integrity check).
        API_EREAD,                ///< File could not be read from (or changed unexpectedly during reading).
        API_EAPPKEY,              ///< Invalid or missing application key.
        API_ESSL,                 ///< SSL verification failed
        API_EGOINGOVERQUOTA,      ///< Not enough quota
        API_EMFAREQUIRED,         ///< Multi-factor authentication required
        API_EMASTERONLY,          ///< Access denied for sub-users (only for business accounts)
        API_EBUSINESSPASTDUE,     ///< Business account expired
        API_EPAYWALL,             ///< Over Disk Quota Paywall

        PAYMENT_ECARD,
        PAYMENT_EBILLING,
        PAYMENT_EFRAUD,
        PAYMENT_ETOOMANY,
        PAYMENT_EBALANCE,
        PAYMENT_EGENERIC ,

        LOCAL_ENOSPC              ///< Insufficient space.
    }
}

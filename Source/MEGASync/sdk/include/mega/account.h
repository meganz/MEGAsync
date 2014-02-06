/**
 * @file mega/account.h
 * @brief Classes for manipulating Account data
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

#ifndef MEGA_ACCOUNT_H
#define MEGA_ACCOUNT_H 1


namespace mega {
// account details/history
struct MEGA_API AccountBalance
{
    double amount;
    char currency[4];
};

struct MEGA_API AccountSession
{
    time_t timestamp, mru;
    string useragent;
    string ip;
    char country[3];
    int current;
};

struct MEGA_API AccountPurchase
{
    time_t timestamp;
    char handle[12];
    char currency[4];
    double amount;
    int method;
};

struct MEGA_API AccountTransaction
{
    time_t timestamp;
    char handle[12];
    char currency[4];
    double delta;
};

struct MEGA_API AccountDetails
{
    // subscription information (summarized)
    int pro_level;
    char subscription_type;

    time_t pro_until;

    // quota related to the session account
    m_off_t storage_used, storage_max;
    m_off_t transfer_own_used, transfer_srv_used, transfer_max;
    m_off_t transfer_own_reserved, transfer_srv_reserved;
    double srv_ratio;

    // transfer history pertaining to requesting IP address
    time_t transfer_hist_starttime;     // transfer history start timestamp
    time_t transfer_hist_interval;      // timespan that a single transfer
                                        // window record covers
    vector<m_off_t> transfer_hist;      // transfer window - oldest to newest,
                                        // bytes consumed per twrtime interval

    m_off_t transfer_reserved;          // byte quota reserved for the
                                        // completion of active transfers

    m_off_t transfer_limit;             // current byte quota for the
                                        // requesting IP address (dynamic,
                                        // overage will be drawn from account
                                        // quota)

    vector<AccountBalance> balances;
    vector<AccountSession> sessions;
    vector<AccountPurchase> purchases;
    vector<AccountTransaction> transactions;
};
} // namespace

#endif

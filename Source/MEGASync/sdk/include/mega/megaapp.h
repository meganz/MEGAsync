/**
 * @file mega/megaapp.h
 * @brief Mega SDK callback interface
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

#ifndef MEGA_APP_H
#define MEGA_APP_H 1

namespace mega {
// callback interface
struct MEGA_API MegaApp
{
    MegaClient* client;

    // a request-level error occurred (other than API_EAGAIN, which will lead
    // to a retry)
    virtual void request_error(error) { }

    // login result
    virtual void login_result(error) { }

    // ephemeral session creation/resumption result
    virtual void ephemeral_result(error) { }
    virtual void ephemeral_result(handle, const byte*) { }

    // account creation
    virtual void sendsignuplink_result(error) { }
    virtual void querysignuplink_result(error) { }
    virtual void querysignuplink_result(handle, const char*, const char*,
                                        const byte*, const byte*, const byte*,
                                        size_t) { }
    virtual void confirmsignuplink_result(error) { }
    virtual void setkeypair_result(error) { }

    // account credentials, properties and history
    virtual void account_details(AccountDetails*, bool, bool, bool, bool, bool, bool) { }
    virtual void account_details(AccountDetails*, error) { }

    // node attribute update failed (not invoked unless error != API_OK)
    virtual void setattr_result(handle, error) { }

    // move node failed (not invoked unless error != API_OK)
    virtual void rename_result(handle, error) { }

    // node deletion failed (not invoked unless error != API_OK)
    virtual void unlink_result(handle, error) { }

    // nodes have been updated
    virtual void nodes_updated(Node**, int) { }

    // users have been added or updated
    virtual void users_updated(User**, int) { }

    // password change result
    virtual void changepw_result(error) { }

    // user attribute update notification
    virtual void userattr_update(User*, int, const char*) { }

    // node fetch result
    virtual void fetchnodes_result(error) { }

    // node addition has failed
    virtual void putnodes_result(error, targettype_t, NewNode*) { }

    // share update result
    virtual void share_result(error) { }
    virtual void share_result(int, error) { }

    // file attribute fetch result
    virtual void fa_complete(Node*, fatype, const char*, uint32_t) { }
    virtual int fa_failed(handle, fatype, int)
    {
        return 0;
    }

    // file attribute modification result
    virtual void putfa_result(handle, fatype, error) { }

    // purchase transactions
    virtual void enumeratequotaitems_result(handle, unsigned, unsigned, unsigned, unsigned, unsigned, const char*) { }
    virtual void enumeratequotaitems_result(error) { }
    virtual void additem_result(error) { }
    virtual void checkout_result(error) { }
    virtual void checkout_result(const char*) { }

    // user invites/attributes
    virtual void invite_result(error) { }
    virtual void putua_result(error) { }
    virtual void getua_result(error) { }
    virtual void getua_result(byte*, unsigned) { }

    // file node export result
    virtual void exportnode_result(error) { }
    virtual void exportnode_result(handle, handle) { }

    // exported link access result
    virtual void openfilelink_result(error) { }
    virtual void openfilelink_result(handle, const byte*, m_off_t, string*, const char*, time_t, time_t, int) { }

    // node opening result
    virtual void checkfile_result(handle, error) { }
    virtual void checkfile_result(handle, error, byte*, m_off_t, time_t, time_t, string*, string*, string*) { }

    // global transfer queue updates (separate signaling towards the queued
    // objects)
    virtual void transfer_added(Transfer*) { }
    virtual void transfer_removed(Transfer*) { }
    virtual void transfer_prepare(Transfer*) { }
    virtual void transfer_failed(Transfer*, error) { }
    virtual void transfer_update(Transfer*) { }
    virtual void transfer_limit(Transfer*) { }
    virtual void transfer_complete(Transfer*) { }

    // sync status updates and events
    virtual void syncupdate_state(Sync*, syncstate_t) { }
    virtual void syncupdate_scanning(bool) { }
    virtual void syncupdate_local_folder_addition(Sync*, const char*) { }
    virtual void syncupdate_local_folder_deletion(Sync*, const char*) { }
    virtual void syncupdate_local_file_addition(Sync*, const char*) { }
    virtual void syncupdate_local_file_deletion(Sync*, const char*) { }
    virtual void syncupdate_local_file_change(Sync*, const char*) { }
    virtual void syncupdate_local_move(Sync*, const char*, const char*) { }
    virtual void syncupdate_local_lockretry(bool) { }
    virtual void syncupdate_get(Sync*, const char*) { }
    virtual void syncupdate_put(Sync*, const char*) { }
    virtual void syncupdate_remote_file_addition(Node*) { }
    virtual void syncupdate_remote_file_deletion(Node*) { }
    virtual void syncupdate_remote_folder_addition(Node*) { }
    virtual void syncupdate_remote_folder_deletion(Node*) { }
    virtual void syncupdate_remote_copy(Sync*, const char*) { }
    virtual void syncupdate_remote_move(string*, string*) { }
    virtual void syncupdate_treestate(LocalNode*) { }

    // sync filename filter
    virtual bool sync_syncable(Node*)
    {
        return true;
    }
    virtual bool sync_syncable(const char*, string*, string*)
    {
        return true;
    }

    // suggest reload due to possible race condition with other clients
    virtual void reload(const char*) { }

    // wipe all users, nodes and shares
    virtual void clearing() { }

    // failed request retry notification
    virtual void notify_retry(dstime) { }

    // generic debug logging
    virtual void debug_log(const char*) { }

    virtual ~MegaApp() { }
};
} // namespace

#endif

/**
 * @file megaclient.cpp
 * @brief sample application, interactive GNU Readline CLI
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

using namespace mega;

extern MegaClient* client;

extern bool debug;

extern void megacli();

extern void term_init();
extern void term_restore();
extern void term_echo(int);

extern void read_pw_char(char*, int, int*, char**);

typedef list<struct AppFile*> appfile_list;

struct AppFile: public File
{
    // app-internal sequence number for queue management
    int seqno;

    bool failed(error);
    void progress();

    appfile_list::iterator appxfer_it;

    AppFile();
};

// application-managed GET and PUT queues (only pending and active files)
extern appfile_list appxferq[2];

struct AppFileGet: public AppFile
{
    void start();
    void update();
    void completed(Transfer*, LocalNode*);

    AppFileGet(Node*, handle = UNDEF, byte* = NULL, m_off_t = -1, time_t = 0, string* = NULL, string* = NULL);
    ~AppFileGet();
};

struct AppFilePut: public AppFile
{
    void start();
    void update();
    void completed(Transfer*, LocalNode*);

    void displayname(string*);

    AppFilePut(string*, handle, const char*);
    ~AppFilePut();
};

struct DemoApp: public MegaApp
{
    FileAccess* newfile();

    void request_error(error);

    void login_result(error);

    void ephemeral_result(error);
    void ephemeral_result(handle, const byte*);

    void sendsignuplink_result(error);
    void querysignuplink_result(error);
    void querysignuplink_result(handle, const char*, const char*, const byte*, const byte*, const byte*, size_t);
    void confirmsignuplink_result(error);
    void setkeypair_result(error);

    void users_updated(User**, int);
    void nodes_updated(Node**, int);
    void nodes_current();

    int prepare_download(Node*);

    void setattr_result(handle, error);
    void rename_result(handle, error);
    void unlink_result(handle, error);

    void fetchnodes_result(error);

    void putnodes_result(error, targettype_t, NewNode*);

    void share_result(error);
    void share_result(int, error);

    void fa_complete(Node*, fatype, const char*, uint32_t);
    int fa_failed(handle, fatype, int);

    void putfa_result(handle, fatype, error);

    void invite_result(error);
    void putua_result(error);
    void getua_result(error);
    void getua_result(byte*, unsigned);

    void account_details(AccountDetails*, bool, bool, bool, bool, bool, bool);
    void account_details(AccountDetails*, error);

    void exportnode_result(error);
    void exportnode_result(handle, handle);

    void openfilelink_result(error);
    void openfilelink_result(handle, const byte*, m_off_t, string*, const char*, time_t, time_t, int);

    void checkfile_result(handle, error);
    void checkfile_result(handle, error, byte*, m_off_t, time_t, time_t, string*, string*, string*);

    void transfer_added(Transfer*);
    void transfer_removed(Transfer*);
    void transfer_prepare(Transfer*);
    void transfer_failed(Transfer*, error);
    void transfer_update(Transfer*);
    void transfer_limit(Transfer*);
    void transfer_complete(Transfer*);

    void syncupdate_state(Sync*, syncstate_t);
    void syncupdate_scanning(bool);
    void syncupdate_local_folder_addition(Sync*, const char*);
    void syncupdate_local_folder_deletion(Sync*, const char*);
    void syncupdate_local_file_addition(Sync*, const char*);
    void syncupdate_local_file_deletion(Sync*, const char*);
    void syncupdate_local_file_change(Sync*, const char*);
    void syncupdate_local_move(Sync*, const char*, const char*);
    void syncupdate_local_lockretry(bool);
    void syncupdate_get(Sync*, const char*);
    void syncupdate_put(Sync*, const char*);
    void syncupdate_remote_file_addition(Node*);
    void syncupdate_remote_file_deletion(Node*);
    void syncupdate_remote_folder_addition(Node*);
    void syncupdate_remote_folder_deletion(Node*);
    void syncupdate_remote_copy(Sync*, const char*);
    void syncupdate_remote_move(string*, string*);
    void syncupdate_treestate(LocalNode*);

    bool sync_syncable(Node*);
    bool sync_syncable(const char*, string*, string*);

    void changepw_result(error);

    void userattr_update(User*, int, const char*);

    void enumeratequotaitems_result(handle, unsigned, unsigned, unsigned, unsigned, unsigned, const char*);
    void enumeratequotaitems_result(error);
    void additem_result(error);
    void checkout_result(error);
    void checkout_result(const char*);

    void reload(const char*);
    void clearing();

    void notify_retry(dstime);
    void debug_log(const char*);
};

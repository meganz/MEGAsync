/*

MEGA SDK 2013-11-11 - sample application, interactive GNU Readline CLI 

(using FreeImage for thumbnail creation)

(c) 2013 by Mega Limited, Wellsford, New Zealand

Applications using the MEGA API must present a valid application key
and comply with the the rules set forth in the Terms of Service.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

extern MegaClient* client;

extern bool debug;

extern bool redisplay;

extern void megacli();

extern void term_init();
extern void term_restore();
extern void term_echo(int);

//extern int globenqueue(const char*, const char*, handle, const char*);

extern void read_pw_char(char*, int, int*, char**);

typedef list<struct AppFile*> appfile_list;

struct AppFile : public File
{
	// app-internal sequence number for queue management
	int seqno;

	bool failed(error);
//	void complete();
	void progress();

	appfile_list::iterator appxfer_it;

	AppFile();
};

// application-managed GET and PUT queues (only pending and active files)
extern appfile_list appxferq[2];

struct AppFileGet : public AppFile
{
	void start();
	void update();
	void completed(Transfer*, LocalNode*);

//	void displayname(string*);

	AppFileGet(Node*);
	~AppFileGet();
};

struct AppFilePut : public AppFile
{
	void start();
	void update();
	void completed(Transfer*, LocalNode*);

	void displayname(string*);
	
	AppFilePut(string*, handle, const char*);
	~AppFilePut();
};

struct DemoApp : public MegaApp
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

	int prepare_download(Node*);

	void setattr_result(handle, error);
	void rename_result(handle, error);
	void unlink_result(handle, error);

	void fetchnodes_result(error);

	void putnodes_result(error, targettype, NewNode*);

	void share_result(error);
	void share_result(int, error);

	void fa_complete(Node*, fatype, const char*, uint32_t);
	int fa_failed(handle, fatype, int);

	void putfa_result(handle, fatype, error);

	void invite_result(error);
	void putua_result(error);
	void getua_result(error);
	void getua_result(byte*, unsigned);

	void account_details(AccountDetails*, int, int, int, int, int, int);
	void account_details(AccountDetails*, error);

	void exportnode_result(error);
	void exportnode_result(handle, handle);

	void openfilelink_result(error);
	void openfilelink_result(Node*);

	void topen_result(int, error);
	void topen_result(int, const char*, int);

	void transfer_added(Transfer*);
	void transfer_removed(Transfer*);
	void transfer_prepare(Transfer*);
	void transfer_failed(Transfer*, error);
	void transfer_update(Transfer*);
	void transfer_limit(Transfer*);
	void transfer_complete(Transfer*);

	void syncupdate_state(Sync*, syncstate);
	void syncupdate_local_folder_addition(Sync*, const char*);
	void syncupdate_local_folder_deletion(Sync*, const char*);
	void syncupdate_local_file_addition(Sync*, const char*);
	void syncupdate_local_file_deletion(Sync*, const char*);
	void syncupdate_get(Sync*, const char*);
	void syncupdate_put(Sync*, const char*);
	void syncupdate_local_mkdir(Sync*, const char*);
	void syncupdate_local_unlink(Node*);
	void syncupdate_local_rmdir(Node*);
	void syncupdate_remote_unlink(Node*);
	void syncupdate_remote_rmdir(Node*);
	void syncupdate_remote_mkdir(Sync*, const char*);

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

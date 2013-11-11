/*

MEGA SDK 2013-10-03 - sample application, interactive GNU Readline CLI 

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

extern int debug;

extern int redisplay;

extern void megacli();

extern void term_init();
extern void term_restore();
extern void term_echo(int);

extern int rename_file(const char*, const char*);
extern int unlink_file(const char*);
extern int change_dir(const char*);

extern int globenqueue(const char*, const char*, handle, const char*);

struct AppFileGet : public FileGet
{
	void start();

	 AppFileGet(handle h) : FileGet(h) { }
};

struct AppFilePut : public FilePut
{
	void start();

	AppFilePut(const char* fn, handle tn, const char* tu, const char* nn) : FilePut(fn,tn,tu,nn) { }
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
	void topen_result(int, string*, const char*, int);

	void transfer_update(int, m_off_t, m_off_t, dstime);
	int transfer_error(int, int, int);
	void transfer_failed(int, error);
	void transfer_failed(int, string&, error);
	void transfer_limit(int);
	void transfer_complete(int, chunkmac_map*, const char*);
	void transfer_complete(int, handle, const byte*, const byte*, SymmCipher*);
	void changepw_result(error);

	void userattr_update(User*, int, const char*);
	
	void reload(const char*);
	void clearing();

	void notify_retry(dstime);
	void debug_log(const char*);
};

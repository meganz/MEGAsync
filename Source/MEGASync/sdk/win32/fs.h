/*

MEGA SDK 2013-11-17 - Win32 filesystem/directory access/notification (UNICODE)

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

#ifndef FSACCESS_CLASS
#define FSACCESS_CLASS WinFileSystemAccess

struct WinDirAccess : public DirAccess
{
	bool ffdvalid;
	WIN32_FIND_DATAW ffd;
	HANDLE hFind;
	string globbase;

public:
	bool dopen(string*, FileAccess*, bool);
	bool dnext(string*, nodetype* = NULL);

	WinDirAccess();
	virtual ~WinDirAccess();
};

class WinFileSystemAccess : public FileSystemAccess
{
public:
	unsigned pendingevents;
	bool notifyerr;

	FileAccess* newfileaccess();
	DirAccess* newdiraccess();

	void tmpnamelocal(string*, string* = NULL);

	void path2local(string*, string*);
	void local2path(string*, string*);

	void name2local(string*, const char* = NULL);
	void local2name(string*);

	bool localhidden(string*, string*);

	bool renamelocal(string*, string*);
	bool copylocal(string*, string*);
	bool rubbishlocal(string*);
	bool unlinklocal(string*);
	bool rmdirlocal(string*);
	bool mkdirlocal(string*);
	bool setmtimelocal(string*, time_t);
	bool chdirlocal(string*);

	void addnotify(LocalNode*, string*);
	void delnotify(LocalNode*);
	bool notifynext(sync_list*, string*, LocalNode**);
	bool notifyfailed();

	void addevents(Waiter*);

	WinFileSystemAccess();
	~WinFileSystemAccess();
};

typedef deque<string> string_deque;

struct WinDirNotify
{
	WinFileSystemAccess* fsaccess;

	HANDLE hDirectory;

	bool active;

	string notifybuf[2];

	string_deque notifyq;

	string basepath;
	LocalNode* localnode;
	string fullpath;

	DWORD dwBytes;
	OVERLAPPED overlapped;

	static VOID CALLBACK completion(DWORD, DWORD, LPOVERLAPPED);

	void process(DWORD wNumberOfBytesTransfered);
	void readchanges();

	WinDirNotify(WinFileSystemAccess*, LocalNode*, string*);
	~WinDirNotify();
};

class WinFileAccess : public FileAccess
{
	HANDLE hFile;

public:
	HANDLE hFind;
	WIN32_FIND_DATAW ffd;

	bool fopen(string*, bool, bool);
	bool fread(string*, unsigned, unsigned, m_off_t);
	bool frawread(byte*, unsigned, m_off_t);
	bool fwrite(const byte*, unsigned, m_off_t);

	WinFileAccess();
	~WinFileAccess();
};

#endif

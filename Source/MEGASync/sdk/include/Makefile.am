libmegaincludedir=$(includedir)

nobase_libmegainclude_HEADERS = \
	mega.h \
	mega/account.h \
	mega/attrmap.h \
	mega/backofftimer.h \
	mega/base64.h \
	mega/config.h \
	mega/console.h \
	mega/command.h \
	mega/db.h \
	mega/fileattributefetch.h \
	mega/filefingerprint.h \
	mega/file.h \
	mega/filesystem.h \
	mega/http.h \
	mega/json.h \
	mega/megaapp.h \
	mega/megaclient.h \
	mega/node.h \
	mega/pubkeyaction.h \
	mega/request.h \
	mega/serialize64.h \
	mega/share.h \
	mega/sharenodekeys.h \
	mega/sync.h \
	mega/transfer.h \
	mega/transferslot.h \
	mega/treeproc.h \
	mega/types.h \
	mega/user.h \
	mega/utils.h \
	mega/waiter.h \
    mega/crypto/cryptopp.h \
    mega/db/sqlite.h \
    mega/db/bdb.h

if WIN32
nobase_libmegainclude_HEADERS += \
	mega/win32/megasys.h \
    mega/win32/megafs.h \
    mega/win32/meganet.h \
    mega/win32/megawaiter.h \
    mega/win32/megaconsole.h \
    mega/win32/megaconsolewaiter.h
else
nobase_libmegainclude_HEADERS += \
	mega/posix/megasys.h \
    mega/posix/megafs.h \
    mega/posix/meganet.h \
    mega/posix/megawaiter.h \
    mega/posix/megaconsole.h \
    mega/posix/megaconsolewaiter.h
endif

#noinst_HEADERS = config.h

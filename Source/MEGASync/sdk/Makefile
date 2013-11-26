CXX=g++
CFLAGS=-DUSE_SQLITE -DUSE_CRYPTOPP -DUSE_INOTIFY -DUSE_FDOPENDIR -g -Wall -I. -Iposix -isystem /usr/include/cryptopp
LIB=-lcryptopp -lfreeimage -lreadline -ltermcap -lcurl -lsqlite3
LDFLAGS=
SOURCES=megacli.cpp megaclient.cpp crypto/cryptopp.cpp posix/fs.cpp posix/console.cpp posix/net.cpp posix/wait.cpp db/sqlite.cpp
OBJECTS=$(SOURCES:.cpp=.o)
PROGS=megacli

all: $(PROGS)

megacli: $(OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIB)

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(PROGS) $(PROGS:=.o)

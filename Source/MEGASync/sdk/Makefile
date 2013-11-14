CXX=g++
CFLAGS=-g -Wall -isystem /usr/include/cryptopp -DENABLE_LOCALCACHE
LIB=-lcryptopp -lfreeimage -lreadline -ltermcap -lcurl -ldb_cxx
LDFLAGS=
SOURCES=megaposix.cpp megacli.cpp megaclient.cpp megacrypto.cpp megabdb.cpp
OBJECTS=$(SOURCES:.cpp=.o)
PROGS=megaclient

all: $(PROGS)

megaclient: $(OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIB)

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(PROGS) $(PROGS:=.o)

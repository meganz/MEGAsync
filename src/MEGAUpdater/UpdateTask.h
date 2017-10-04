#ifndef UPDATETASK_H
#define UPDATETASK_H

#include "megaapi.h"

#include <cryptopp/cryptlib.h>
#include <cryptopp/modes.h>
#include <cryptopp/ccm.h>
#include <cryptopp/gcm.h>
#include <cryptopp/integer.h>
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/crc.h>
#include <cryptopp/nbtheory.h>
#include <cryptopp/algparam.h>
#include <cryptopp/hmac.h>
#include <cryptopp/pwdbased.h>

class Base64
{
    static byte to64(byte);
    static byte from64(byte);

public:
    static int btoa(const std::string&, std::string&);
    static int btoa(const byte*, int, char*);
    static int atob(const std::string&, std::string&);
    static int atob(const char*, byte*, int);
};

class SignatureChecker
{
public:
    SignatureChecker(const char *base64Key);
    ~SignatureChecker();

    void init();
    void add(const char *data, unsigned size);
    bool checkSignature(const char *base64Signature);

protected:
    CryptoPP::Integer key[2];
    CryptoPP::SHA512 hash;
};

class UpdateTask
{
public:
    explicit UpdateTask(mega::MegaApi *megaApi);
    ~UpdateTask();
    void checkForUpdates();

protected:
    bool downloadFile(std::string url, std::string dstPath);
    bool processUpdateFile(FILE *fd);
    bool fileExist(const char* path);
    void initSignature();
    void addToSignature(const char *bytes, int length);
    bool checkSignature(std::string value);
    bool alreadyInstalled(std::string relativePath, std::string fileSignature);
    bool alreadyDownloaded(std::string relativePath, std::string fileSignature);
    bool alreadyExists(std::string absolutePath, std::string fileSignature);
    bool performUpdate();
    void rollbackUpdate(int fileNum);
    void initialCleanup();
    void finalCleanup();
    bool removeRecursively(std::string path);
    int readVersion();
    std::string getAppDataDir();
    std::string readNextLine(FILE *fd);

    std::string appFolder;
    std::string appDataFolder;
    std::string updateFolder;
    std::string backupFolder;
    SignatureChecker *signatureChecker;
    mega::MegaApi *megaApi;
    mega::SynchronousRequestListener *delegateListener;
    int currentFile;
    int updateVersion;
    std::vector<std::string> downloadURLs;
    std::vector<std::string> localPaths;
    std::vector<std::string> fileSignatures;
};

#endif // UPDATETASK_H

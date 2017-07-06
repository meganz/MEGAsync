#ifndef UPDATETASK_H
#define UPDATETASK_H

#include "megaapi.h"

class UpdateTask
{

public:

    explicit UpdateTask(mega::MegaApi *megaApi);
    ~UpdateTask();

public:
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

    std::string updateFolder;
    std::string backupFolder;
    mega::MegaHashSignature *signatureChecker;
    mega::MegaApi *megaApi;
    mega::SynchronousRequestListener *delegateListener;
    int currentFile;
    int updateVersion;
    std::vector<std::string> downloadURLs;
    std::vector<std::string> localPaths;
    std::vector<std::string> fileSignatures;
};

#endif // UPDATETASK_H

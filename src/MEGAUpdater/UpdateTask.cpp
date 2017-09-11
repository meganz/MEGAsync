#include "UpdateTask.h"
#include "Preferences.h"
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctime>
#include <sstream>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>

using namespace mega;
using namespace std;

#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
#include <direct.h>
#include <io.h>
#include <algorithm>

#ifndef PATH_MAX
    #define PATH_MAX _MAX_PATH
#endif

#define MEGA_SEPARATOR '\\'
#define mega_mkdir(x) _mkdir(x)
#define mega_access(x) _access(x, 0)

string UpdateTask::getAppDataDir()
{
    string path;
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(GetModuleFileName(NULL, szPath , MAX_PATH))
            && SUCCEEDED(PathRemoveFileSpec(szPath)))
    {
        MegaApi::utf16ToUtf8(szPath, lstrlen(szPath), &path);
        path.append("\\");
    }
    return path;
}

#define MEGA_DATA_FOLDER appDataFolder
#define MEGA_TO_NATIVE_SEPARATORS(x) std::replace(x.begin(), x.end(), '/', '\\');
#define MEGA_SET_PERMISSIONS

#else

#define MEGA_SEPARATOR '/'
#define mega_mkdir(x) mkdir(x, S_IRWXU)
#define mega_access(x) access(x, F_OK)

string UpdateTask::getAppDataDir()
{
    string path;
    const char* home = getenv("HOME");
    if (home)
    {
        path.append(home);
        path.append("/Library/Application\ Support/Mega\ Limited/MEGAsync/");
    }
    return path;
}

#define MEGA_DATA_FOLDER APP_DIR_BUNDLE
#define MEGA_TO_NATIVE_SEPARATORS(x) std::replace(x.begin(), x.end(), '\\', '/');
#define MEGA_SET_PERMISSIONS chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGAclient", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

#endif

#define mega_base_path(x) x.substr(0, x.find_last_of("/\\") + 1)

#define MAX_LOG_SIZE 1024
char log_message[MAX_LOG_SIZE];
#define LOG(logLevel, ...) snprintf(log_message, MAX_LOG_SIZE, __VA_ARGS__); \
                                   MegaApi::log(logLevel, log_message);
int mkdir_p(const char *path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p;
    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path) - 1)
    {
        return -1;
    }
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++)
    {
        if (*p == '\\' || *p == '/')
        {
            /* Temporarily truncate */
            *p = '\0';
            if (mega_mkdir(_path) != 0 && errno != EEXIST)
            {
                return -1;
            }
            *p = MEGA_SEPARATOR;
        }
    }

    if (mega_mkdir(_path) != 0 && errno != EEXIST)
    {
        return -1;
    }
    return 0;
}

UpdateTask::UpdateTask(MegaApi *megaApi)
{
    this->megaApi = megaApi;
    signatureChecker = new MegaHashSignature((const char *)UPDATE_PUBLIC_KEY);
    delegateListener = new SynchronousRequestListener();
    currentFile = -1;
    appDataFolder = getAppDataDir();
    appFolder = MEGA_DATA_FOLDER;
    updateFolder = appDataFolder + UPDATE_FOLDER_NAME + MEGA_SEPARATOR;
    backupFolder = appDataFolder + BACKUP_FOLDER_NAME + MEGA_SEPARATOR;
}

UpdateTask::~UpdateTask()
{
    delete delegateListener;
    delete signatureChecker;
}

void UpdateTask::checkForUpdates()
{
    LOG(MegaApi::LOG_LEVEL_INFO, "Starting update check");
    initialCleanup();

    // Create random sequence for http request
    string randomSec("?");
    for (int i = 0; i < 10; i++)
    {
        randomSec += char('A'+ (rand() % 26));
    }

    string appData = appDataFolder;
    string updateFile = appData.append(UPDATE_FILENAME);
    if (downloadFile((char *)((string(UPDATE_CHECK_URL) + randomSec).c_str()), updateFile.c_str()))
    {
        FILE * pFile;
        pFile = fopen(updateFile.c_str(), "r");
        if (!pFile)
        {
            LOG(MegaApi::LOG_LEVEL_ERROR, "Error opening update file");
            return;
        }

        if (!processUpdateFile(pFile))
        {
            return;
        }

        currentFile++;
        while ((size_t)currentFile < downloadURLs.size())
        {
            if (!alreadyDownloaded(localPaths[currentFile], fileSignatures[currentFile]))
            {
                //Create the folder for the new file
                string localFile = updateFolder + localPaths[currentFile];
                if (mkdir_p(mega_base_path(localFile).c_str()) == -1)
                {
                    LOG(MegaApi::LOG_LEVEL_INFO, "Unable to create folder for file: %s", localFile.c_str());
                    return;
                }

                //Delete the file if exists
                if (fileExist(localFile.c_str()))
                {
                    unlink(localFile.c_str());
                }

                //Download file to specific folder
                if (downloadFile(downloadURLs[currentFile], localFile))
                {
                    LOG(MegaApi::LOG_LEVEL_INFO, "File downloaded OK: %s", localPaths[currentFile].c_str());
                    if (!alreadyDownloaded(localPaths[currentFile], fileSignatures[currentFile]))
                    {
                        LOG(MegaApi::LOG_LEVEL_ERROR, "Signature of downloaded file doesn't match: %s",  localPaths[currentFile].c_str());
                        return;
                    }
                    LOG(MegaApi::LOG_LEVEL_INFO, "File signature OK: %s",  localPaths[currentFile].c_str());
                    currentFile++;
                    continue;
                }
                return;
            }

            LOG(MegaApi::LOG_LEVEL_INFO, "File already downloaded: %s",  localPaths[currentFile].c_str());
            currentFile++;
        }

        //All files have been processed. Apply update
        if (!performUpdate())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Error applying update");
            return;
        }

        finalCleanup();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Unable to download file");
        return;
    }
}

bool UpdateTask::downloadFile(string url, string dstPath)
{
    LOG(MegaApi::LOG_LEVEL_INFO, "Downloading updated file from: %s",  url.c_str());
    megaApi->downloadFile((char*)url.c_str(), (char*)dstPath.c_str(), delegateListener);
    delegateListener->wait();

    int errorCode = delegateListener->getError()->getErrorCode();
    if (errorCode != MegaError::API_OK)
    {
        LOG(MegaApi::LOG_LEVEL_ERROR, "Unable to download file. Error code: %d", errorCode);
        return true;
    }

    return true;
}

bool UpdateTask::processUpdateFile(FILE *fd)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Reading update info");
    string version = readNextLine(fd);
    if (version.empty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Invalid update info");
        return false;
    }

    int currentVersion = readVersion();
    if (currentVersion == -1)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,"Error reading file version (megasync.version)");
        return false;
    }

    updateVersion = atoi(version.c_str());
    if (updateVersion <= currentVersion)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,"Update not needed");
        return false;
    }

    string updateSignature = readNextLine(fd);
    if (updateSignature.empty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty info signature)");
        return false;
    }

    initSignature();
    addToSignature(version.data(), version.length());

    while (true)
    {
        string url = readNextLine(fd);
        if (url.empty())
        {
            break;
        }

        string localPath = readNextLine(fd);
        if (localPath.empty())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty path)");
            return false;
        }

        string fileSignature = readNextLine(fd);
        if (fileSignature.empty())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty file signature)");
            return false;
        }

        addToSignature(url.data(), url.length());
        addToSignature(localPath.data(), localPath.length());
        addToSignature(fileSignature.data(), fileSignature.length());

        MEGA_TO_NATIVE_SEPARATORS(localPath);
        if (alreadyInstalled(localPath, fileSignature))
        {
            LOG(MegaApi::LOG_LEVEL_INFO, "File already installed: %s",  localPath.c_str());
            continue;
        }

        downloadURLs.push_back(url);
        localPaths.push_back(localPath);
        fileSignatures.push_back(fileSignature);
    }

    if (!downloadURLs.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "All files are up to date");
        return false;
    }

    if (!checkSignature(updateSignature))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (invalid signature)");
        return false;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Update needed");
    return true;
}

bool UpdateTask::fileExist(const char *path)
{
    return (mega_access(path) != -1);
}

void UpdateTask::addToSignature(const char* bytes, int length)
{
    signatureChecker->add(bytes, length);
}

void UpdateTask::initSignature()
{
    signatureChecker->init();
}

bool UpdateTask::checkSignature(string value)
{
    bool result = signatureChecker->checkSignature(value.c_str());
    if (!result)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Invalid signature");
    }

    return result;
}

bool UpdateTask::performUpdate()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Applying update...");
    for (vector<string>::size_type i = 0; i < localPaths.size(); i++)
    {
        string file = backupFolder + localPaths[i];
        if (mkdir_p(mega_base_path(file).c_str()) == -1)
        {
            LOG(MegaApi::LOG_LEVEL_ERROR, "Error creating backup folder for: %s",  file.c_str());
            rollbackUpdate(i);
            return false;
        }

        string origFile = appFolder + localPaths[i];
        if (rename(origFile.c_str(), file.c_str()) && errno != ENOENT)
        {
            LOG(MegaApi::LOG_LEVEL_ERROR, "Error creating backup of file %s to %s",  origFile.c_str(), file.c_str());
            rollbackUpdate(i);
            return false;
        }

        if (mkdir_p(mega_base_path(origFile).c_str()) == -1)
        {
            LOG(MegaApi::LOG_LEVEL_ERROR, "Error creating target folder for: %s",  origFile.c_str());
            rollbackUpdate(i);
            return false;
        }

        string update = updateFolder + localPaths[i];
        if (rename(update.c_str(), origFile.c_str()))
        {
            LOG(MegaApi::LOG_LEVEL_ERROR, "Error installing file %s in %s",  update.c_str(), origFile.c_str());
            rollbackUpdate(i);
            return false;
        }

        LOG(MegaApi::LOG_LEVEL_INFO, "File correctly installed: %s",  localPaths[i].c_str());
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update successfully installed");
    return true;
}

void UpdateTask::rollbackUpdate(int fileNum)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Uninstalling update...");
    for (int i = fileNum; i >= 0; i--)
    {
        string origFile = appFolder + localPaths[i];
        rename(origFile.c_str(), (updateFolder + localPaths[i]).c_str());
        rename((backupFolder + localPaths[i]).c_str(), origFile.c_str());
        LOG(MegaApi::LOG_LEVEL_INFO, "File restored: %s",  localPaths[i].c_str());
    }
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update uninstalled");
}

void UpdateTask::initialCleanup()
{
    removeRecursively(backupFolder);
}

void UpdateTask::finalCleanup()
{
    initialCleanup();
    remove(appDataFolder.append(UPDATE_FILENAME).c_str());
    MEGA_SET_PERMISSIONS;
}

bool UpdateTask::removeRecursively(string path)
{
    MegaApi::removeRecursively(path.c_str());
    return !rmdir(path.c_str());
}

bool UpdateTask::alreadyInstalled(string relativePath, string fileSignature)
{
    return alreadyExists(appFolder + relativePath, fileSignature);
}

bool UpdateTask::alreadyDownloaded(string relativePath, string fileSignature)
{
    return alreadyExists(updateFolder + relativePath, fileSignature);
}

bool UpdateTask::alreadyExists(string absolutePath, string fileSignature)
{
    MegaHashSignature tmpHash((const char *)UPDATE_PUBLIC_KEY);
    char *buffer;
    long fileLength;
    FILE * pFile = fopen(absolutePath.c_str(), "rb");
    if (pFile == NULL)
    {
        return false;
    }

    //Get size of file and rewind FILE pointer
    fseek(pFile, 0, SEEK_END);
    fileLength = ftell(pFile);
    rewind(pFile);

    buffer = (char *)malloc(fileLength);
    if (buffer == NULL)
    {
        return false;
    }

    size_t sizeRead = fread(buffer, 1, fileLength, pFile);
    if (sizeRead != fileLength)
    {
        return false;
    }

    tmpHash.add(buffer, sizeRead);
    fclose(pFile);
    free(buffer);

    return tmpHash.checkSignature(fileSignature.data());
}

string UpdateTask::readNextLine(FILE *fd)
{
    char line[4096];
    if (!fgets(line, sizeof(line), fd))
    {
        return string();
    }

    line[strcspn(line, "\n")] = '\0';
    return string(line);
}

int UpdateTask::readVersion()
{
    int version = -1;
    FILE *fp = fopen(appDataFolder.append(VERSION_FILE_NAME).c_str(), "r");
    if (fp == NULL)
    {
        return version;
    }

    fscanf(fp, "%d", &version);
    fclose(fp);
    return version;
}

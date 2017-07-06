#include "UpdateTask.h"
#include "Preferences.h"
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctime>
#include <sstream>
#include <cstdio>

using namespace mega;
using namespace std;

char separator()
{
#ifdef _WIN32
    return '\\')
#else
    return '/';
#endif
}

std::string base_name(std::string const & path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

std::string base_path(std::string const & path)
{
    return path.substr(0, path.find_last_of("/\\") + 1);
}

int mkdir_p(const char *path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p;
    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path)-1)
    {
        return -1;
    }
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++)
    {
        if (*p == '/')
        {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0)
            {
                if (errno != EEXIST)
                {
                    return -1;
                }
            }

            *p = '/';
        }
    }

    if (mkdir(_path, S_IRWXU) != 0)
    {
        if (errno != EEXIST)
        {
            return -1;
        }
    }

    return 0;
}

UpdateTask::UpdateTask(MegaApi *megaApi)
{
    this->megaApi = megaApi;
    updateFolder = getAppDataDir() + UPDATE_FOLDER_NAME;
    updateFolder += separator();
    backupFolder = getAppDataDir() + BACKUP_FOLDER_NAME;
    backupFolder += separator();
    currentFile = -1;
    signatureChecker = new MegaHashSignature((const char *)UPDATE_PUBLIC_KEY);
    delegateListener = new SynchronousRequestListener();
}

UpdateTask::~UpdateTask()
{
    delete delegateListener;
    delete signatureChecker;
}

void UpdateTask::checkForUpdates()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Starting update check");
    initialCleanup();

    // Create random sequence for http request
    string randomSec("?");
    for (int i = 0; i < 10; i++)
    {
        randomSec += char('A'+(rand() % 26));
    }

    string updateFile = getAppDataDir().append(UPDATE_FILENAME);
    if (downloadFile((char *)((string(UPDATE_CHECK_URL) + randomSec).c_str()), updateFile.c_str()))
    {
        FILE * pFile;
        pFile = fopen(updateFile.c_str(), "r");
        if (pFile == NULL)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Error opening update file");
            return;
        }
        if (!processUpdateFile(pFile))
        {
            return;
        }

        currentFile++;
        while (currentFile < downloadURLs.size())
        {
            if (!alreadyDownloaded(localPaths[currentFile], fileSignatures[currentFile]))
            {
                //Create the folder for the new file
                string localFile = updateFolder + localPaths[currentFile];
                if (mkdir_p(base_path(localFile).c_str()) == -1)
                {
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
                    currentFile++;
                    continue;
                }
                return;
            }

            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "File already downloaded:");
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, localPaths[currentFile].c_str());
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
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Downloading updated file from:");
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, url.c_str());

    megaApi->downloadFile((char*)url.c_str(), (char*)dstPath.c_str(), delegateListener);
    delegateListener->wait();
    if (delegateListener->getError()->getErrorCode() == MegaError::API_OK)
    {
        return true;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Unable to download file");
    return false;
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

        if (alreadyInstalled(localPath, fileSignature))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, localPath.c_str());
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

    return true;
}

bool UpdateTask::fileExist(const char *path)
{
    if(access(path, F_OK) != -1 )
    {
        return true;
    }
    return false;
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
    int result = signatureChecker->checkSignature(value.c_str());
    if (!result)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Invalid signature");
    }

    return result;
}

bool UpdateTask::performUpdate()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Applying update...");

    time_t now = time(0);
    tm *timeData = localtime(&now);
    ostringstream oss;
    oss <<  timeData->tm_mday << "_" << 1 + timeData->tm_mon << "_" << 1900 + timeData->tm_year;
    oss <<  timeData->tm_hour << "_" << timeData->tm_min << "_" << timeData->tm_sec;
    oss << separator();
    backupFolder += oss.str();

    for (int i = 0; i < localPaths.size(); i++)
    {

        string file = backupFolder + localPaths[i];
        if (mkdir_p(base_path(file).c_str()) == -1)
        {
            return false;
        }

        string origFile = APP_DIR_BUNDLE + localPaths[i];
        int result = rename(origFile.c_str() , file.c_str());
        if (result == 0)
        {
            int error =  errno;
        }

        string update = updateFolder + localPaths[i];
        if (rename(update.c_str(), origFile.c_str()) != 0)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error installing file: ");
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, origFile.c_str());
            rollbackUpdate(i);
            return false;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "File installed: ");
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, localPaths[i].c_str());

    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update successfully installed");
    return true;
}

void UpdateTask::rollbackUpdate(int fileNum)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Uninstalling update...");
    for (int i = fileNum; i >= 0; i--)
    {
        string origFile = APP_DIR_BUNDLE + localPaths[i];
        rename(origFile.c_str(), (updateFolder + localPaths[i]).c_str());
        rename((backupFolder + localPaths[i]).c_str(), origFile.c_str());

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "File restored: ");
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, localPaths[i].c_str());
    }
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update uninstalled");
}


void UpdateTask::initialCleanup()
{
    removeRecursively(updateFolder);
    removeRecursively(backupFolder);
}

void UpdateTask::finalCleanup()
{
    initialCleanup();
    remove(getAppDataDir().append(UPDATE_FILENAME).c_str());
    chmod("/Applications/MEGAsync.app/Contents/MacOS/MEGAclient", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}

bool UpdateTask::removeRecursively(string path)
{
    if (path.empty())
    {
        return false;
    }

    bool success = false;
    mega::MegaApi::removeRecursively(path.c_str());
    success = rmdir(path.c_str());
    return !success;
}

bool UpdateTask::alreadyInstalled(string relativePath, string fileSignature)
{
    return alreadyExists(APP_DIR_BUNDLE + relativePath, fileSignature);
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

    FILE * pFile;
    pFile = fopen(absolutePath.c_str(), "r");
    if (pFile == NULL)
    {
        return false;
    }

    //Get size of file and rewind FILE pointer
    fseek(pFile, 0, SEEK_END);
    fileLength = ftell(pFile);
    rewind(pFile);

    buffer = (char *) malloc ((fileLength) * sizeof(char));
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
    bool check = tmpHash.checkSignature(fileSignature.data());

    fclose(pFile);
    free(buffer);

    return check;
}

string UpdateTask::readNextLine(FILE *fd)
{
    char line[4096];
    if (!fgets(line, sizeof(line), fd))
    {
        return string();
    }

    line[strcspn(line, "\n")] = '\0';
    string qLine(line);

    return qLine; //Fix me. Trim me both sides ><

}


int UpdateTask::readVersion()
{
    int version = -1;
    FILE *fp = fopen(getAppDataDir().append(VERSION_FILE_NAME).c_str(), "r");
    if (fp == NULL)
    {
        return version;
    }

    fscanf(fp, "%d", &version);
    fclose(fp);
    return version;
}

string UpdateTask::getAppDataDir()
{
    string dir;
    const char* home = getenv("HOME");
    if (home)
    {
        dir.append(home);
        dir.append("/Library/Application\ Support/Mega\ Limited/MEGAsync/");
        return dir;
    }
}

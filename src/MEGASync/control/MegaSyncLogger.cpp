#include "MegaSyncLogger.h"
#include "Utilities.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <assert.h>

#include <QFileInfo>
#include <QString>
#include <QDesktopServices>
#include <QDir>
#include <QFile>


#include <chrono>
#include <thread>
#include <condition_variable>

#include <zlib.h>

#include <megaapi.h>

#define MEGA_LOGGER QString::fromUtf8("MEGA_LOGGER")
#define ENABLE_MEGASYNC_LOGS QString::fromUtf8("MEGA_ENABLE_LOGS")
#define MAX_MESSAGE_SIZE 4096

#define MAX_FILESIZE_MB 10    // 10MB of log usually compresses to about 850KB (was 450 before duplicate line detection) 
#define MAX_ROTATE_LOGS 50   // So we expect to keep 42MB or so in compressed logs
#define MAX_ROTATE_LOGS_TODELETE 50   // If ever reducing the number of logs, we should remove the older ones anyway. This number should be the historical maximum of that value


#ifdef _WIN32
    #define CERRQSTRING(filename) std::wcerr << filename.toStdU16String()
#else
    #define CERRQSTRING(filename) std::cerr << filename.toUtf8().constData()
#endif


void gzipCompressOnRotate(const QString filename, const QString destinationFilename)
{
#ifdef WIN32
    std::ifstream file((wchar_t*)filename.toStdU16String().data(), std::ofstream::out);
#else
    std::ifstream file(filename.toUtf8().data(), std::ofstream::out);
#endif
    if (!file.is_open())
    {
        std::cerr << "Unable to open log file for reading: "; CERRQSTRING(filename) << std::endl;
        return;
    }

    auto gzdeleter = [](gzFile_s* f) { if (f) gzclose(f); };

#ifdef _WIN32
    std::unique_ptr<gzFile_s, decltype(gzdeleter)> gzfile{ gzopen_w((wchar_t*)destinationFilename.toStdU16String().data(), "wb"), gzdeleter};
#else
    std::unique_ptr<gzFile_s, decltype(gzdeleter)> gzfile{ gzopen(destinationFilename.toUtf8().data(), "wb"), gzdeleter };
#endif
    if (!gzfile)
    {
        std::cerr << "Unable to open gzfile for writing: "; CERRQSTRING(filename) << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        line.push_back('\n');
        if (gzputs(gzfile.get(), line.c_str()) == -1)
        {
            std::cerr << "Unable to compress log file: "; CERRQSTRING(filename) << std::endl;
            return;
        }
    }

    gzfile.reset();
    file.close();
    QFile::remove(filename);
}

struct LogLinkedList
{
    LogLinkedList* next = nullptr;
    unsigned allocated = 0;
    unsigned used = 0;
    int lastmessage = -1;
    int lastmessageRepeats = 0;
    char message[1];

    static LogLinkedList* create(LogLinkedList* prev, size_t size)
    {
        LogLinkedList* entry = (LogLinkedList*)malloc(size);
        entry->next = nullptr;
        entry->allocated = unsigned(size - sizeof(LogLinkedList));
        entry->used = 0;
        entry->lastmessage = -1;
        entry->lastmessageRepeats = 0;
        prev->next = entry;
        return entry;
    }

    bool messageFits(size_t size)
    {
        return used + size + 2 < allocated;
    }

    void append(const char* s)
    {
        unsigned n = unsigned(strlen(s));
        assert(used + n + 1 < allocated);
        strcpy(message + used, s);
        used += n;
    }

};


std::unique_ptr<std::thread> logThread;
std::condition_variable logConditionVariable;
std::mutex logMutex;
std::mutex logRotationMutex;
LogLinkedList logListFirst;
LogLinkedList* logListLast = &logListFirst;
bool logExit = false;
bool flushLog = false;
bool forceRotationForReporting = false;
bool logToDesktop = false;
bool logToDesktopChanged = false;
int flushOnLevel = mega::MegaApi::LOG_LEVEL_WARNING;
std::chrono::seconds logFlushPeriod = std::chrono::seconds(10);
std::chrono::steady_clock::time_point nextFlushTime = std::chrono::steady_clock::now() + logFlushPeriod;

MegaSyncLogger *megaSyncLogger = nullptr;

QString numberedLogFilename(QString baseName, int logNumber)
{
    QString newName = baseName;
    auto index = newName.lastIndexOf(QString::fromUtf8("."));
    if (index > 0) newName = newName.left(index + 1);
    newName += QString::number(logNumber) + QString::fromUtf8(".log");
    return newName;
}



void logThreadFunction(QString filename, QString desktopFilename)
{
#ifdef WIN32
    std::ofstream outputFile((wchar_t*)filename.toStdU16String().data(), std::ofstream::out | std::ofstream::app);
#else
    std::ofstream outputFile(filename.toUtf8().data(), std::ofstream::out | std::ofstream::app);
#endif
    outputFile << "----------------------------- program start -----------------------------" << endl;
    long long outFileSize = outputFile.tellp();
    std::ofstream logDesktopFile;

    while (!logExit)
    {
        if (forceRotationForReporting || outFileSize > MAX_FILESIZE_MB*1024*1024)
        {
            logRotationMutex.lock();
            for (int i = MAX_ROTATE_LOGS_TODELETE; i--; )
            {
                QString toRename = numberedLogFilename(filename, i);

                if (QFile::exists(toRename))
                {
                    if (i + 1 >= MAX_ROTATE_LOGS)
                    {
                        if (!QFile::remove(toRename))
                        {
                            std::cerr << "Error removing log file " << i << std::endl;
                        }

                    }
                    else
                    {
                        if (!QFile(toRename).rename(numberedLogFilename(filename, i + 1)))
                        {
                            std::cerr << "Error renaming log file " << i << std::endl;
                        }
                    }
                }
            }
            auto newNameDone = numberedLogFilename(filename, 0);
            auto newNameZipping = newNameDone + QString::fromUtf8(".zipping");

            outputFile.close();
            QFile::remove(newNameZipping);
            QFile(filename).rename(newNameZipping);

            bool report = forceRotationForReporting;
            forceRotationForReporting = false;

            std::thread t([=]() {
                gzipCompressOnRotate(newNameZipping, newNameDone); 
                logRotationMutex.unlock();
                if (report && megaSyncLogger)
                {
                    emit megaSyncLogger->logReadyForReporting();
                }
            });
            t.detach();

#ifdef WIN32
            outputFile.open((wchar_t*)filename.toStdU16String().data(), std::ofstream::out);
#else
            outputFile.open(filename.toUtf8().data(), std::ofstream::out);
#endif
            outFileSize = 0;
        }

        LogLinkedList* newMessages = nullptr;

        {
            std::unique_lock<std::mutex> lock(logMutex);
            logConditionVariable.wait_for(lock, std::chrono::milliseconds(500), [&newMessages]() { 
                    if (logListFirst.next || logExit || forceRotationForReporting || logToDesktopChanged)
                    {
                        newMessages = logListFirst.next;
                        logListFirst.next = nullptr;
                        logListLast = &logListFirst;
                        return true;
                    }
                    else return false;
            });
        }

        while (newMessages)
        {
            auto p = newMessages;
            newMessages = newMessages->next;
            if (outputFile)
            {
                outputFile << p->message;
                outFileSize += p->used;
            }
            if (logDesktopFile)
            {
                logDesktopFile << p->message;
            }
            free(p);
        }
        if (flushLog || forceRotationForReporting || nextFlushTime <= std::chrono::steady_clock::now())
        {
            flushLog = false;
            outputFile.flush();
            nextFlushTime = std::chrono::steady_clock::now() + logFlushPeriod;
        }

        if (logToDesktopChanged)
        {
            logToDesktopChanged = false;
            if (logToDesktop && !logDesktopFile)
            {
#ifdef WIN32
                logDesktopFile.open((wchar_t*)desktopFilename.toStdU16String().data(), std::ofstream::out | std::ofstream::app);
#else
                logDesktopFile.open(desktopFilename.toUtf8().data(), std::ofstream::out | std::ofstream::app);
#endif
            }
            else if (!logToDesktop && logDesktopFile)
            {
                logDesktopFile.close();
            }
        }

    }
}


MegaSyncLogger::MegaSyncLogger(QObject *parent, const QString& dataPath, const QString& desktopPath, bool logToStdout)
: QObject{parent}
, mDesktopPath{desktopPath}
{
    megaSyncLogger = this;

    const QDir dataDir{dataPath};
    dataDir.mkdir(LOGS_FOLDER_LEAFNAME_QSTRING);
    const auto logPath = dataDir.filePath(LOGS_FOLDER_LEAFNAME_QSTRING + QString::fromUtf8("/MEGAsync.log"));

    const QDir desktopDir{mDesktopPath};
    const auto desktopLogPath = desktopDir.filePath(QString::fromUtf8("MEGAsync.log"));

    if (!logThread)
    {
        logThread.reset(new std::thread([logPath, desktopLogPath]() {
            logThreadFunction(logPath, desktopLogPath);
        }));
    }

    mega::MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_MAX);
    mega::MegaApi::addLoggerObject(this);
}

MegaSyncLogger::~MegaSyncLogger()
{
    mega::MegaApi::removeLoggerObject(this); // after this no more calls to MegaSyncLogger::log

    {
        std::lock_guard<std::mutex> g(logMutex);
        logExit = true;
        logConditionVariable.notify_one();
    }
    megaSyncLogger = nullptr;
    logThread->join();
    logThread.reset();
}

inline void twodigit(char*& s, int n)
{
    *s++ = n / 10 + '0';
    *s++ = n % 10 + '0';
}

char* filltime(char* s, struct tm*  gmt, int millisec)
{
    // strftime was seen in 1.27% of profiler stack samples with constant logging, try manual
    // this version only seen in 0.06% of profiler stack samples
    twodigit(s, gmt->tm_mon + 1);
    *s++ = '/';
    twodigit(s, gmt->tm_mday);
    *s++ = '-';
    twodigit(s, gmt->tm_hour);
    *s++ = ':';
    twodigit(s, gmt->tm_min);
    *s++ = ':';
    twodigit(s, gmt->tm_sec);
    *s++ = '.';
    *s++ = millisec / 100 + '0';
    *s++ = (millisec / 10) % 10 + '0';
    *s++ = millisec % 10 + '0';
    *s++ = ' ';
    *s = 0;
    return s;
}

std::mutex threadNameMutex;
std::map<std::thread::id, std::string> threadNames;
struct tm lastTm;
time_t lastT = 0;
std::thread::id lastThreadId;
const char* lastThreadName;

void cacheThreadNameAndTimeT(time_t t, struct tm& gmt, const char*& threadname)
{
    std::lock_guard<std::mutex> g(threadNameMutex);

    if (t != lastT)
    {
        lastTm = *std::gmtime(&t);
        lastT = t;
    }
    gmt = lastTm;

    if (lastThreadId == std::this_thread::get_id())
    {
        threadname = lastThreadName;
        return;
    }

    auto& entry = threadNames[std::this_thread::get_id()];
    if (entry.empty())
    {
        std::ostringstream s;
        s << std::this_thread::get_id() << " ";
        entry = s.str();
    }
    threadname = lastThreadName = entry.c_str();
    lastThreadId = std::this_thread::get_id();
}

void MegaSyncLogger::log(const char*, int loglevel, const char*, const char *message)
{


// todo: do we need this xml logger?
//#ifdef LOG_TO_LOGGER
//    //if (mConnected)
//    {
//        QString m = QString::fromUtf8(message);
//        if (m.size() > MAX_MESSAGE_SIZE)
//        {
//            m = m.left(MAX_MESSAGE_SIZE - 3).append(QString::fromUtf8("..."));
//        }
//
//        auto t = std::time(NULL);
//        char ts[150];
//        if (!std::strftime(ts, sizeof(ts), "%H:%M:%S", std::gmtime(&t)))    // .%f nonstandard, not available on windows
//        {
//            ts[0] = '\0';
//        }
//        emit sendLog(QString::fromUtf8(ts), loglevel, m);
//    }
//#endif


    char timebuf[30];
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);

    struct tm gmt;
    const char* threadname;
    cacheThreadNameAndTimeT(t, gmt, threadname);

    auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::system_clock::from_time_t(t));
    filltime(timebuf, &gmt, (int)millisec.count());

    const char* loglevelstring = "     ";
    switch (loglevel) // keeping these at 4 chars makes nice columns, easy to read
    {
    case mega::MegaApi::LOG_LEVEL_FATAL: loglevelstring = "CRIT "; break;
    case mega::MegaApi::LOG_LEVEL_ERROR: loglevelstring = "ERR  "; break;
    case mega::MegaApi::LOG_LEVEL_WARNING: loglevelstring = "WARN "; break;
    case mega::MegaApi::LOG_LEVEL_INFO: loglevelstring = "INFO "; break;
    case mega::MegaApi::LOG_LEVEL_DEBUG: loglevelstring = "DBG  "; break;
    case mega::MegaApi::LOG_LEVEL_MAX: loglevelstring = "DTL  "; break;
    }
    
    auto messageLen = strlen(message);
    auto lineLen = strlen(timebuf) + strlen(threadname) + strlen(loglevelstring) + messageLen;
    bool notify = false;

    {
        std::lock_guard<std::mutex> g(logMutex);

        bool isRepeat = logListLast != &logListFirst && 
                        logListLast->lastmessage >= 0 && 
                        !strncmp(message, logListLast->message + logListLast->lastmessage, messageLen);

        if (isRepeat)
        {
            ++logListLast->lastmessageRepeats;
        }
        else
        {
            unsigned reportRepeats = logListLast != &logListFirst ? logListLast->lastmessageRepeats : 0;
            if (reportRepeats)
            {
                lineLen += 30;
                logListLast->lastmessageRepeats = 0;
            }
            if (logListLast == &logListFirst || !logListLast->messageFits(lineLen))
            {
                logListLast = LogLinkedList::create(logListLast, std::max<size_t>(lineLen, 8192) + sizeof(LogLinkedList) + 10);
            }
            if (reportRepeats)
            {
                char repeatbuf[31]; // this one can occur very frequently with many in a row: cURL DEBUG: schannel: failed to decrypt data, need more data
                snprintf(repeatbuf, 30, "[repeated x%u]\n", reportRepeats);
                logListLast->append(repeatbuf);
            }
            logListLast->append(timebuf);
            logListLast->append(threadname);
            logListLast->append(loglevelstring);
            logListLast->lastmessage = logListLast->used;
            logListLast->append(message); 
            logListLast->append("\n");
            notify = logListLast->used + 1024 > logListLast->allocated;
        }

        if (loglevel <= flushOnLevel)
        {
            flushLog = true;
        }
    }
    
    if (notify)
    {
        // notify outside the mutex lock is better (and correct) for much less chance the other 
        // thread wakes up just to find the mutex locked. (saw lower cpu on the other thread like this)
        // Still, this notify call was taking 1% when notifying on every log line, so let the other thead
        // wake up by itself every 500ms without notify for the common case.
        // But still wake it if our memory block is getting full
        logConditionVariable.notify_one();
    }
}

void MegaSyncLogger::setDebug(const bool enable)
{
    logToDesktop = enable;
    logToDesktopChanged = true;
}

bool MegaSyncLogger::isDebug() const
{
    return logToDesktop;
}

bool MegaSyncLogger::prepareForReporting()
{
    std::lock_guard<std::mutex> g(logMutex);
    forceRotationForReporting = true;
    logConditionVariable.notify_one();
    return true;
}

void MegaSyncLogger::resumeAfterReporting()
{
}


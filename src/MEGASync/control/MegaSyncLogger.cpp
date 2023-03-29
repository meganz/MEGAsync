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
#include <future>

#ifdef WIN32
#include <windows.h>
#endif

//#define MEGA_LOGGER QString::fromUtf8("MEGA_LOGGER")
//#define ENABLE_MEGASYNC_LOGS QString::fromUtf8("MEGA_ENABLE_LOGS")
#define MAX_MESSAGE_SIZE 4096

#define LOG_TIME_CHARS 22
#define LOG_LEVEL_CHARS 5

#define MAX_LOG_FILESIZE_MB_DEFAULT 10    // 10MB of log usually compresses to about 850KB (was 450 before duplicate line detection)
#define MAX_ROTATE_LOGS_DEFAULT 50   // So we expect to keep 42MB or so in compressed logs
#define MAX_ROTATE_LOGS_TODELETE 50   // If ever reducing the number of logs, we should remove the older ones anyway. This number should be the historical maximum of that value


#ifdef _WIN32
    #define CERRQSTRING(filename) std::wcerr << filename.toStdWString()
#else
    #define CERRQSTRING(filename) std::cerr << filename.toUtf8().constData()
#endif


void gzipCompressOnRotate(const QString filename, const QString destinationFilename)
{
#ifdef WIN32
    std::ifstream file(filename.toStdWString().data(), std::ofstream::out);
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
    std::unique_ptr<gzFile_s, decltype(gzdeleter)> gzfile{ gzopen_w(destinationFilename.toStdWString().data(), "wb"), gzdeleter};
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

using DirectLogFunction = std::function <void (std::ostream *)>;

struct LogLinkedList
{
    LogLinkedList* next = nullptr;
    unsigned allocated = 0;
    unsigned used = 0;
    int lastmessage = -1;
    int lastmessageRepeats = 0;
    bool oomGap = false;
    DirectLogFunction *mDirectLoggingFunction = nullptr; // we cannot use a non pointer due to the malloc allocation of new entries
    std::promise<void>* mCompletionPromise = nullptr; // we cannot use a unique_ptr due to the malloc allocation of new entries
    char message[1];

    static LogLinkedList* create(LogLinkedList* prev, size_t size)
    {
        LogLinkedList* entry = (LogLinkedList*)malloc(size);
        if (entry)
        {
            entry->next = nullptr;
            entry->allocated = unsigned(size - sizeof(LogLinkedList));
            entry->used = 0;
            entry->lastmessage = -1;
            entry->lastmessageRepeats = 0;
            entry->oomGap = false;
            entry->mDirectLoggingFunction = nullptr;
            entry->mCompletionPromise = nullptr;
            prev->next = entry;
        }
        return entry;
    }

    bool messageFits(size_t size)
    {
        return used + size + 2 < allocated;
    }

    bool needsDirectOutput()
    {
        return mDirectLoggingFunction != nullptr;
    }

    void append(const char* s, unsigned int n = 0)
    {
        n = n ? n : unsigned(strlen(s));
        assert(used + n + 1 < allocated);
        strcpy(message + used, s);
        used += n;
    }

    void notifyWaiter()
    {
        if (mCompletionPromise)
        {
            mCompletionPromise->set_value();
        }
    }

};

MegaSyncLogger *g_megaSyncLogger = nullptr;

struct LoggingThread
{
    std::unique_ptr<std::thread> logThread;
    std::condition_variable logConditionVariable;
    std::mutex logMutex;
    std::mutex logRotationMutex;
    LogLinkedList logListFirst;
    LogLinkedList* logListLast = &logListFirst;
    bool logExit = false;
    bool flushLog = false;
    bool closeLog = false;
    bool forceRotationForReporting = false;
    bool forceRenew = false; //to force removal of all logs and create an empty MEGAsync.log
    bool logToDesktop = false;
    bool logToDesktopChanged = false;
    int flushOnLevel = mega::MegaApi::LOG_LEVEL_WARNING;
    std::chrono::seconds logFlushPeriod = std::chrono::seconds(10);
    std::chrono::steady_clock::time_point nextFlushTime = std::chrono::steady_clock::now() + logFlushPeriod;

    void startLoggingThread(QString filename, QString desktopFilename)
    {
        if (!logThread)
        {
            logThread.reset(new std::thread([this, filename, desktopFilename]() {
                logThreadFunction(filename, desktopFilename);
            }));
        }
    }

    void log(int loglevel, const char *message, const char **directMessages = nullptr, size_t *directMessagesSizes = nullptr, int numberMessages = 0);

private:
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
        int logSizeBeforeCompressMb = MAX_LOG_FILESIZE_MB_DEFAULT;
        if (auto mb = getenv("MEGA_MAX_LOG_FILESIZE_MB"))
        {
            logSizeBeforeCompressMb = atoi(mb);
        }
        int logCountToRotate = MAX_ROTATE_LOGS_DEFAULT;
        int logCountToClean = MAX_ROTATE_LOGS_TODELETE;
        if (auto count = getenv("MEGA_MAX_ROTATE_LOGS"))
        {
            logCountToRotate = atoi(count);
            logCountToClean = std::max(logCountToRotate, logCountToClean);
        }

    #ifdef WIN32
        std::ofstream outputFile(filename.toStdWString().data(), std::ofstream::out | std::ofstream::app);
    #else
        std::ofstream outputFile(filename.toUtf8().data(), std::ofstream::out | std::ofstream::app);
    #endif
        outputFile << "----------------------------- program start -----------------------------\n";
        long long outFileSize = outputFile.tellp();
        std::ofstream logDesktopFile;
        bool logDesktopFileOpen = false;

        while (!logExit)
        {
            if (forceRenew)
            {
                std::lock_guard<std::mutex> g(logRotationMutex);
                for (int i = logCountToClean; i--; )
                {
                    QString toDelete = numberedLogFilename(filename, i);

                    if (QFile::exists(toDelete))
                    {
                        if (!QFile::remove(toDelete))
                        {
                            std::cerr << "Error removing log file " << i << std::endl;
                        }
                    }
                }

                outputFile.close();
                if (!QFile::remove(filename) )
                {
                    std::cerr << "Error removing log file!! " << std::endl;
                }

    #ifdef WIN32
                outputFile.open(filename.toStdWString().data(), std::ofstream::out);
    #else
                outputFile.open(filename.toUtf8().data(), std::ofstream::out);
    #endif
                outFileSize = 0;

                forceRenew = false;

                if (g_megaSyncLogger)
                {
                    emit g_megaSyncLogger->logCleaned();
                }
            }
            else if (forceRotationForReporting || outFileSize > logSizeBeforeCompressMb*1024*1024)
            {
                std::lock_guard<std::mutex> g(logRotationMutex);
                for (int i = logCountToClean; i--; )
                {
                    QString toRename = numberedLogFilename(filename, i);

                    if (QFile::exists(toRename))
                    {
                        if (i + 1 >= logCountToRotate)
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
                    std::lock_guard<std::mutex> g(logRotationMutex); // prevent another rotation while we work on this file (in case of unfortunate timing with bug report etc)
                    gzipCompressOnRotate(newNameZipping, newNameDone);
                    if (report && g_megaSyncLogger)
                    {
                        emit g_megaSyncLogger->logReadyForReporting();
                    }
                });
                t.detach();

    #ifdef WIN32
                outputFile.open(filename.toStdWString().data(), std::ofstream::out);
    #else
                outputFile.open(filename.toUtf8().data(), std::ofstream::out);
    #endif
                outFileSize = 0;
            }

            LogLinkedList* newMessages = nullptr;
            bool topLevelMemoryGap = false;
            {
                std::unique_lock<std::mutex> lock(logMutex);
                logConditionVariable.wait_for(lock, std::chrono::milliseconds(500), [this, &newMessages, &topLevelMemoryGap]() {
                        if (forceRenew || logListFirst.next || logExit || forceRotationForReporting || logToDesktopChanged || flushLog || closeLog)
                        {
                            newMessages = logListFirst.next;
                            logListFirst.next = nullptr;
                            logListLast = &logListFirst;
                            topLevelMemoryGap = logListFirst.oomGap;
                            logListFirst.oomGap = false;
                            return true;
                        }
                        else return false;
                });
            }

            if (logToDesktopChanged)
            {
                logToDesktopChanged = false;
                if (logToDesktop && !logDesktopFileOpen)
                {
    #ifdef WIN32
                    logDesktopFile.open(desktopFilename.toStdWString().data(), std::ofstream::out | std::ofstream::app);
    #else
                    logDesktopFile.open(desktopFilename.toUtf8().data(), std::ofstream::out | std::ofstream::app);
    #endif
                    logDesktopFileOpen = true;
                }
                else if (!logToDesktop && logDesktopFileOpen)
                {
                    logDesktopFile.close();
                    logDesktopFileOpen = false;
                }
            }

            if (topLevelMemoryGap)
            {
                if (outputFile)
                {
                    outputFile << "<log gap - out of logging memory at this point>\n";
                }
                if (logDesktopFile)
                {
                    logDesktopFile << "<log gap - out of logging memory at this point>\n";
                }
            }

            while (newMessages)
            {
                auto p = newMessages;
                newMessages = newMessages->next;
                if (outputFile)
                {
                    if (p->needsDirectOutput())
                    {
                        (*p->mDirectLoggingFunction)(&outputFile);
                    }
                    else
                    {
                        outputFile << p->message;
                        outFileSize += p->used;
                        if (p->oomGap)
                        {
                            outputFile << "<log gap - out of logging memory at this point>\n";
                        }
                    }
                }
                if (logDesktopFile)
                {
                    if (p->needsDirectOutput())
                    {
                        (*p->mDirectLoggingFunction)(&logDesktopFile);
                    }
                    else
                    {
                        logDesktopFile << p->message;
                        if (p->oomGap)
                        {
                            logDesktopFile << "<log gap - out of logging memory at this point>\n";
                        }
                    }
                    if (!newMessages)
                    {
                        logDesktopFile.flush(); //always flush in `active` logging
                    }
                }

                if (g_megaSyncLogger && g_megaSyncLogger->mLogToStdout)
                {
                    if (p->needsDirectOutput())
                    {
                        (*p->mDirectLoggingFunction)(&std::cout);
                    }
                    else
                    {
                        std::cout << p->message;
                    }
                    if (!newMessages)
                    {
                        std::cout << std::flush; //always flush into stdout (DEBUG mode)
                    }
                }
                p->notifyWaiter();
                free(p);
            }
            if (flushLog || forceRotationForReporting || nextFlushTime <= std::chrono::steady_clock::now())
            {
                flushLog = false;
                outputFile.flush();
                if (logDesktopFile)
                {
                    logDesktopFile.flush();
                }
                if (g_megaSyncLogger && g_megaSyncLogger->mLogToStdout)
                {
                    std::cout << std::flush;
                }
                nextFlushTime = std::chrono::steady_clock::now() + logFlushPeriod;
            }

            if (closeLog)
            {
                outputFile.close();
                if (logDesktopFile)
                {
                    logDesktopFile.close();
                }
                return;  // This request means we have received a termination signal; close and exit the thread as quick & clean as possible
            }
        }
    }

};


MegaSyncLogger::MegaSyncLogger(QObject *parent, const QString& dataPath, const QString& desktopPath, bool logToStdout)
: QObject{parent}
, mDesktopPath{desktopPath}
{
    assert(!g_megaSyncLogger);
    g_megaSyncLogger = this;
    mLogToStdout = logToStdout;

    const QDir dataDir{dataPath};
    dataDir.mkdir(LOGS_FOLDER_LEAFNAME_QSTRING);
    const auto logPath = dataDir.filePath(LOGS_FOLDER_LEAFNAME_QSTRING + QString::fromUtf8("/MEGAsync.log"));

    const QDir desktopDir{mDesktopPath};
    const auto desktopLogPath = desktopDir.filePath(QString::fromUtf8("MEGAsync.log"));

    g_loggingThread.reset(new LoggingThread());
    g_loggingThread->startLoggingThread(logPath, desktopLogPath);

    mega::MegaApi::setLogLevel(mega::MegaApi::LOG_LEVEL_MAX);
    mega::MegaApi::addLoggerObject(this, true);
}

MegaSyncLogger::~MegaSyncLogger()
{
    // any other threads that might be logging have to be shut down before we call this
    mega::MegaApi::removeLoggerObject(this, true);

    {
        std::lock_guard<std::mutex> g(g_loggingThread->logMutex);
        g_loggingThread->logExit = true;
        g_loggingThread->logConditionVariable.notify_one();
    }
    assert(g_megaSyncLogger == this);
    g_megaSyncLogger = nullptr;
    g_loggingThread->logThread->join();
    g_loggingThread->logThread.reset();
}

inline void twodigit(char*& s, int n)
{
    *s++ = static_cast<char>(n / 10 + '0');
    *s++ = static_cast<char>(n % 10 + '0');
}

char* filltime(char* s, struct tm*  gmt, int microsec)
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

    s[5] = static_cast<char>(microsec % 10 + '0');
    s[4] = static_cast<char>((microsec /= 10) % 10 + '0');
    s[3] = static_cast<char>((microsec /= 10) % 10 + '0');
    s[2] = static_cast<char>((microsec /= 10) % 10 + '0');
    s[1] = static_cast<char>((microsec /= 10) % 10 + '0');
    s[0] = static_cast<char>((microsec /= 10) % 10 + '0');
    s += 6;
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

void MegaSyncLogger::log(const char*, int loglevel, const char*, const char *message
#ifdef ENABLE_LOG_PERFORMANCE
                         , const char **directMessages, size_t *directMessagesSizes, int numberMessages
#endif
                         )

{
    g_loggingThread->log(loglevel, message
#ifdef ENABLE_LOG_PERFORMANCE
                        , directMessages, directMessagesSizes, numberMessages
#endif
                        );
}

void LoggingThread::log(int loglevel, const char *message, const char **directMessages, size_t *directMessagesSizes, int numberMessages)
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

    bool direct = directMessages != nullptr;

    char timebuf[LOG_TIME_CHARS + 1];
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);

    struct tm gmt;
    const char* threadname;
    cacheThreadNameAndTimeT(t, gmt, threadname);

    auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(now - std::chrono::system_clock::from_time_t(t));
    filltime(timebuf, &gmt, (int)microsec.count() % 1000000);

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
    auto threadnameLen = strlen(threadname);
    auto lineLen = LOG_TIME_CHARS + threadnameLen + LOG_LEVEL_CHARS + messageLen;
    bool notify = false;

    {
        std::unique_ptr<std::lock_guard<std::mutex>> g(new std::lock_guard<std::mutex>(logMutex));

        bool isRepeat = !direct && logListLast != &logListFirst &&
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

#if defined(WIN32) && defined(DEBUG)
            OutputDebugStringA(std::string(timebuf).c_str());
            OutputDebugStringA(std::string(threadname).c_str());
            OutputDebugStringA(std::string(loglevelstring).c_str());
            if (message)
            {
                OutputDebugStringA(std::string(message, messageLen).c_str());
            }
            for(int i = 0; i < numberMessages; i++)
            {
                OutputDebugStringA(std::string(directMessages[i], directMessagesSizes[i]).c_str());
            }
            OutputDebugStringA("\r\n");
#endif
            if (direct)
            {
                if (LogLinkedList* newentry = LogLinkedList::create(logListLast, 1 + sizeof(LogLinkedList))) //create a new "empty" element
                {
                    logListLast = newentry;
                    std::promise<void> promise;
                    logListLast->mCompletionPromise = &promise;
                    auto future = logListLast->mCompletionPromise->get_future();
                    DirectLogFunction func = [&timebuf, &threadname, &loglevelstring, &directMessages, &directMessagesSizes, numberMessages](std::ostream *oss)
                    {
                        *oss << timebuf << threadname << loglevelstring;

                        for(int i = 0; i < numberMessages; i++)
                        {
                            oss->write(directMessages[i], directMessagesSizes[i]);
                        }
                        *oss << std::endl;
                    };

                    logListLast->mDirectLoggingFunction = &func;

                    g.reset(); //to liberate the mutex and let the logging thread call the logging function

                    logConditionVariable.notify_one();

                    //wait for until logging thread completes the outputting
                    future.get();
                    return;
                }
                else
                {
                    logListLast->oomGap = true;
                }

            }
            else
            {
                if (logListLast == &logListFirst || logListLast->oomGap || !logListLast->messageFits(lineLen))
                {
                    if (LogLinkedList* newentry = LogLinkedList::create(logListLast, std::max<size_t>(lineLen, 8192) + sizeof(LogLinkedList) + 10))
                    {
                        logListLast = newentry;
                    }
                    else
                    {
                        logListLast->oomGap = true;
                    }
                }
                if (!logListLast->oomGap)
                {
                    if (reportRepeats)
                    {
                        char repeatbuf[31]; // this one can occur very frequently with many in a row: cURL DEBUG: schannel: failed to decrypt data, need more data
                        int n = snprintf(repeatbuf, 30, "[repeated x%u]\n", reportRepeats);
                        logListLast->append(repeatbuf, n);
                    }
                    logListLast->append(timebuf, LOG_TIME_CHARS);
                    logListLast->append(threadname, unsigned(threadnameLen));
                    logListLast->append(loglevelstring, LOG_LEVEL_CHARS);
                    logListLast->lastmessage = logListLast->used;
                    logListLast->append(message, unsigned(messageLen));
                    logListLast->append("\n", 1);
                    notify = logListLast->used + 1024 > logListLast->allocated;
                }
            }
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
    g_loggingThread->logToDesktop = enable;
    g_loggingThread->logToDesktopChanged = true;
}

bool MegaSyncLogger::isDebug() const
{
    return g_loggingThread->logToDesktop;
}

bool MegaSyncLogger::prepareForReporting()
{
    std::lock_guard<std::mutex> g(g_loggingThread->logMutex);
    g_loggingThread->forceRotationForReporting = true;
    g_loggingThread->logConditionVariable.notify_one();
    return true;
}

bool MegaSyncLogger::cleanLogs()
{
    std::lock_guard<std::mutex> g(g_loggingThread->logMutex);
    g_loggingThread->forceRenew = true;
    g_loggingThread->logConditionVariable.notify_one();
    return true;
}

void MegaSyncLogger::resumeAfterReporting()
{
}

void MegaSyncLogger::flushAndClose()
{
    try
    {
        g_loggingThread->log(mega::MegaApi::LOG_LEVEL_FATAL, "***CRASH DETECTED: FLUSHING AND CLOSING***");

    }
    catch (const std::exception& e)
    {
        std::cerr << "Unhandle exception on flushAndClose: "<< e.what() << std::endl;
    }
    g_loggingThread->flushLog = true;
    g_loggingThread->closeLog = true;
    g_loggingThread->logConditionVariable.notify_one();
    // This is called on crash so the app may be unstable. Don't assume the thread is working properly.
    // It might be the one that crashed.  Just give it 1 second to complete
#ifdef WIN32
    Sleep(1000);
#else
    usleep(1000000);
#endif
}



#include "MegaSyncLogger.h"
#include <iostream>
#include <sstream>

#include <QFileInfo>
#include <QString>
#include <QDesktopServices>
#include <QDir>

using namespace mega;
using namespace std;

MegaSyncLogger::MegaSyncLogger() : QObject(), MegaLogger()
{
#ifdef LOG_TO_LOGGER
    xmlWriter = NULL;
    connect(this, SIGNAL(sendLog(QString,int,QString)), this, SLOT(onLogAvailable(QString,int,QString)));
#endif
}

void MegaSyncLogger::log(const char *time, int loglevel, const char *source, const char *message)
{
    QString fileName;
    QFileInfo info(QString::fromUtf8(source));
    fileName = info.fileName();

#ifdef LOG_TO_LOGGER
    emit sendLog(QString::fromUtf8(time), loglevel, QString::fromUtf8(message));
#endif

#if defined(LOG_TO_FILE) || defined(LOG_TO_STDOUT)
    ostringstream oss;
    oss << time;
    switch(loglevel)
    {
        case MegaApi::LOG_LEVEL_DEBUG:
            oss << " (debug): ";
            break;
        case MegaApi::LOG_LEVEL_ERROR:
            oss << " (error): ";
            break;
        case MegaApi::LOG_LEVEL_FATAL:
            oss << " (fatal): ";
            break;
        case MegaApi::LOG_LEVEL_INFO:
            oss << " (info):  ";
            break;
        case MegaApi::LOG_LEVEL_MAX:
            oss << " (verb):  ";
            break;
        case MegaApi::LOG_LEVEL_WARNING:
            oss << " (warn):  ";
            break;
    }

    oss << message;
    if(fileName.size())
        oss << " ("<< fileName.toStdString() << ")";

    #ifdef LOG_TO_STDOUT
        cout << oss.str() << endl;
    #endif

    #ifdef LOG_TO_FILE
        static QString filePath;
        if(filePath.isEmpty())
        {
            QString dataPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
            filePath = dataPath + QDir::separator() + QString::fromAscii("MEGAsync.log");
        }

        QFile file(filePath);
        file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream out(&file);
        out << oss.str().c_str() << endl;
    #endif
#endif
}

void MegaSyncLogger::onLogAvailable(QString time, int loglevel, QString message)
{
    if(client.state() == QLocalSocket::ClosingState)
    {
        client.waitForDisconnected(1000);
        return;
    }

    if(client.state() == QLocalSocket::UnconnectedState)
    {
        client.connectToServer(QString::fromUtf8("MEGA_SERVER"));
        return;
    }

    if(client.state() == QLocalSocket::ConnectingState)
    {
        if(!client.waitForConnected(1000))
        {
            QLocalSocket::LocalSocketError e = client.error();
            cout << "E: "<< e << endl;
        }
        return;
    }

    //QLocalSocket::ConnectedState
    if(xmlWriter && xmlWriter->hasError())
    {
        delete xmlWriter;
        xmlWriter = NULL;
    }

    if(!xmlWriter)
    {
        xmlWriter = new QXmlStreamWriter(&client);
        xmlWriter->setAutoFormatting(true);
        xmlWriter->writeStartDocument();
        xmlWriter->writeStartElement(QString::fromUtf8("msgs"));
    }

    QString level;
    switch(loglevel)
    {
        case MegaApi::LOG_LEVEL_DEBUG:
            level = QString::fromUtf8("debug");
            break;
        case MegaApi::LOG_LEVEL_ERROR:
            level = QString::fromUtf8("error");
            break;
        case MegaApi::LOG_LEVEL_FATAL:
            level = QString::fromUtf8("fatal");
            break;
        case MegaApi::LOG_LEVEL_INFO:
            level = QString::fromUtf8("info");
            break;
        case MegaApi::LOG_LEVEL_MAX:
            level = QString::fromUtf8("verbose");
            break;
        case MegaApi::LOG_LEVEL_WARNING:
            level = QString::fromUtf8("warning");
            break;
        default:
            level = QString::fromUtf8("unknown");
            break;
    }


    xmlWriter->writeStartElement(QString::fromUtf8("msg"));
    xmlWriter->writeTextElement(QString::fromUtf8("timestamp"), time);
    xmlWriter->writeTextElement(QString::fromUtf8("type"),level);
    xmlWriter->writeTextElement(QString::fromUtf8("content"), message);
    xmlWriter->writeEndElement();

    client.flush();
}
